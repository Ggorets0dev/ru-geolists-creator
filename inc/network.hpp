#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <curl/curl.h>

#include "json_io.hpp"

#define DOWNLOAD_TRY_DELAY_SEC      1u

extern void
downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

extern bool
downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

extern bool
isUrlAccessible(const std::string& url);

#endif // NETWORK_HPP
