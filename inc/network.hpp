#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

#include "log.hpp"
#include "json_io.hpp"

#define DOWNLOAD_TRY_DELAY_SEC      1u

extern bool
downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

extern bool
downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

#endif // NETWORK_HPP
