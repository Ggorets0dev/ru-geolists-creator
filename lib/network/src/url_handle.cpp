#include "url_handle.hpp"
#include "log.hpp"
#include "exception.hpp"
#include "libnetwork_settings.hpp"

#include <curl/curl.h>
#include <fstream>
#include <algorithm>
#include <thread>
#include <regex>

#define GITHUB_TOKEN_HEADER             "Authorization: Bearer "

#define USER_AGENT                      "RGLC"

static size_t writeToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;

    if (outFile->is_open()) {
        outFile->write(static_cast<char*>(contents), totalSize);
    }

    return totalSize;
}

static bool isUrlAccessible(const std::string& url, const char* httpHeader = nullptr) {
	CURL *curl = curl_easy_init();
    CURLcode res;
    volatile uint32_t responseCode(0);

    if (!curl) {
        curl_easy_cleanup(curl);
        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, gLibNetworkSettings.curlOperationTimeoutSec);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gLibNetworkSettings.curlConnectionTimeoutSec);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

    if (httpHeader != nullptr && strlen(httpHeader) > 0) {
        struct curl_slist *header = nullptr;
        header = curl_slist_append(header, httpHeader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    }

    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    }

    curl_easy_cleanup(curl);

    return (res == CURLE_OK && responseCode >= 200 && responseCode < 400);
}

static void downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr) {
    CURL* curl;
    CURLcode res;
    bool isAccessed;

    isAccessed = NetUtils::tryAccessUrl(url, httpHeader);

    if (!isAccessed) {
        throw CurlError("Failed to access URL in download handler", CURLE_COULDNT_CONNECT);
    }

    // Open file for write
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + filePath);
    }

    curl = curl_easy_init();
    if (curl) {
        // Set url settings
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

        // Set callback for write
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, gLibNetworkSettings.curlOperationTimeoutSec);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gLibNetworkSettings.curlConnectionTimeoutSec);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

        if (httpHeader != nullptr && strlen(httpHeader) > 0) {
            struct curl_slist *header = nullptr;
            header = curl_slist_append(header, httpHeader);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        }

        // Perform query
        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        outFile.close();

        // Check for errors
        if (res != CURLE_OK) {
            throw CurlError("Failed to download file", res);
        }
    } else {
        curl_easy_cleanup(curl);
        outFile.close();

        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }
}

static std::string genGithubTokenHeader(const std::string& token) {
    return GITHUB_TOKEN_HEADER + token;
}

bool NetUtils::tryAccessUrl(const std::string& url, const char* httpHeader) {
    for(unsigned int i(0); i < gLibNetworkSettings.connectAttemptsCount; ++i) {
        if (isUrlAccessible(url, httpHeader)) {
            return true;
        }

        LOG_WARNING("Failed to access URL, performing another attempt...");
        std::this_thread::sleep_for(std::chrono::seconds(gLibNetworkSettings.connectAttemptDelaySec));
    }

    return false;
}

bool NetUtils::tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken) {
    std::string tokenHeader = genGithubTokenHeader(apiToken);

    if (!apiToken.empty()) {
        return tryDownloadFile(url, filePath, tokenHeader.c_str());
    } else {
        return tryDownloadFile(url, filePath);
    }
}

bool NetUtils::tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader) {
    for(unsigned int i(0); i < gLibNetworkSettings.downloadAttemptCount; ++i) {
        try {
            downloadFile(url, filePath, httpHeader);
        }  catch (std::exception& e) {
            LOG_ERROR(e.what());
            LOG_WARNING("Failed to download requested file, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(gLibNetworkSettings.downloadAttemptDelaySec));

            continue;
        }

        return true;
    }

    return false;
}

bool NetUtils::downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames) {
    bool status;

    if (value.isMember("assets") && value["assets"].isArray()) {
        const Json::Value& assets = value["assets"];
        for (const auto& asset : assets) {
            if (std::find(fileNames.begin(), fileNames.end(), asset["name"].asString()) == fileNames.end()) {
                continue;
            }

            status = tryDownloadFile(asset["browser_download_url"].asString(), asset["name"].asString());

            if (!status) {
                return status;
            }
        }
    }

    return true;
}

NetTypes::AddressType NetUtils::getAddressType(const std::string& input) {
    try {
        // IPv4 with mask /0–32
        static const std::regex kPatternIPv4(
            R"(^(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d))"
            R"((/(3[0-2]|[12]?\d))?$)"
        );

        // IPv6 (simplified, ECMAScript-compatible)
        static const std::regex kPatternIPv6(
            R"(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"
        );

        static const std::regex kPatternDomain(
            // Доменное имя: одна или более меток, разделённых точкой
            // Каждая метка: 1–63 символа, не начинается и не заканчивается дефисом
            // TLD: только буквы, минимум 2 символа
            R"(^((?![0-9-])[A-Za-z0-9-]{0,62}[A-Za-z0-9]\.)*(?![0-9-])[A-Za-z0-9-]{0,62}[A-Za-z0-9]\.[A-Za-z]{2,63}$)"
        );

        if (std::regex_match(input, kPatternIPv4)) {
            return NetTypes::AddressType::IPV4;
        }
        if (std::regex_match(input, kPatternIPv6)) {
            return NetTypes::AddressType::IPV6;
        }
        if (std::regex_match(input, kPatternDomain) || input == "localhost") {
            return NetTypes::AddressType::DOMAIN;
        }

        return NetTypes::AddressType::UNKNOWN;
    }
    catch (const std::regex_error& e) {
        LOG_ERROR("Regex error occured after trying to get address type: ", e.what());
        return NetTypes::AddressType::UNKNOWN;
    }
}