#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <curl/curl.h>

#include "json_io.hpp"

#define DOWNLOAD_TRY_DELAY_SEC      1u

void downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

bool isUrlAccessible(const std::string& url, const char* httpHeader = nullptr);

#endif // NETWORK_HPP
