#include "network.hpp"
#include "exception.hpp"
#include "log.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>

#define GITHUB_TOKEN_HEADER         "Authorization: Bearer "

#define USER_AGENT                  "ru-geolists-creator"

#define CURL_OPERATION_TIMEOUT_SEC      10u
#define CURL_CONNECTION_TIMEOUT_SEC     5u

#define CONNECT_ATTEMPTS_COUNT          3u
#define CONNECT_ATTEMPT_DELAY_SEC       2u

#define DOWNLOAD_ATTEMPT_COUNT          3u
#define DOWNLOAD_ATTEMPT_DELAY_SEC      2u

static size_t writeToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_OPERATION_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECTION_TIMEOUT_SEC);
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

    isAccessed = tryAccessUrl(url, httpHeader);

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

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_OPERATION_TIMEOUT_SEC);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECTION_TIMEOUT_SEC);

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

bool tryAccessUrl(const std::string& url, const char* httpHeader) {
    for(uint8_t i(0); i < CONNECT_ATTEMPTS_COUNT; ++i) {
        if (isUrlAccessible(url, httpHeader)) {
            return true;
        } else {
            LOG_WARNING("Failed to access URL, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(CONNECT_ATTEMPT_DELAY_SEC));

            continue;
        }
    }

    return false;
}

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken) {
    std::string tokenHeader = genGithubTokenHeader(apiToken);

    if (!apiToken.empty()) {
        return tryDownloadFile(url, filePath, tokenHeader.c_str());
    } else {
        return tryDownloadFile(url, filePath);
    }
}

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader) {
    for(uint8_t i(0); i < DOWNLOAD_ATTEMPT_COUNT; ++i) {
        try {
            downloadFile(url, filePath, httpHeader);
        }  catch (std::exception& e) {
            LOG_ERROR(e.what());
            LOG_WARNING("Failed to download requested file, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(DOWNLOAD_ATTEMPT_DELAY_SEC));

            continue;
        }

        return true;
    }

    return false;
}

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames) {
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
