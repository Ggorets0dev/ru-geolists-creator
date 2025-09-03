#include "network.hpp"
#include "exception.hpp"
#include "log.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>

#define USER_AGENT "ru-geolists-creator"

static size_t
writeToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;

    if (outFile->is_open()) {
        outFile->write(static_cast<char*>(contents), totalSize);
    }

    return totalSize;
}

bool
isUrlAccessible(const std::string& url) {
	CURL *curl = curl_easy_init();
    CURLcode res;
    long responseCode(0);

    if (!curl) {
        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    }

    curl_easy_cleanup(curl);

    return (res == CURLE_OK && responseCode >= 200 && responseCode < 400);
}

void
downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader) {
    CURL* curl;
    CURLcode res;

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

        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

        if (httpHeader != nullptr && strlen(httpHeader) > 0) {
            struct curl_slist *header = nullptr;
            header = curl_slist_append(header, httpHeader);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        }

        // Perform query
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            throw CurlError("Failed to download file", res);
        }

        // Free curl resources
        curl_easy_cleanup(curl);
    } else {
        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }

    outFile.close();
}

bool
downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames) {
    if (value.isMember("assets") && value["assets"].isArray()) {
        const Json::Value& assets = value["assets"];
        for (const auto& asset : assets) {
            if (std::find(fileNames.begin(), fileNames.end(), asset["name"].asString()) == fileNames.end()) {
                continue;
            }

            try {
                downloadFile(asset["browser_download_url"].asString(), asset["name"].asString());
            }  catch (std::exception& e) {
                LOG_ERROR(e.what());

                return false;
            }
        }
    }

    return true;
}
