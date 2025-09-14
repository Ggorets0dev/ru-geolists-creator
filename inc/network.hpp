#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <curl/curl.h>

#include "json_io.hpp"

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

bool tryAccessUrl(const std::string& url, const char* httpHeader = nullptr);

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken);

#endif // NETWORK_HPP
