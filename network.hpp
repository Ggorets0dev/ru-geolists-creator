#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

#include "log.hpp"

#define DOWNLOAD_TRY_DELAY_SEC      1u

extern bool
downloadFile(const std::string& url, const std::string& filePath);

#endif // NETWORK_HPP
