#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <curl/curl.h>
#include <optional>
#include <algorithm>

#include "log.hpp"
#include "json_io.hpp"

#define DOWNLOAD_TRY_DELAY_SEC      1u

extern bool
downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

extern std::optional<std::time_t>
parsePublishTime(const Json::Value& value);

extern bool
downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

#endif // NETWORK_HPP
