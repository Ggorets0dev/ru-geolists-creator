#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <curl/curl.h>
#include <set>
#include <bitset>

#include "json_io.hpp"

namespace NetTypes {
    using rawIPv4 = std::bitset<32>;
    using rawIPv6 = std::bitset<128>;

    struct IPv4 {
        rawIPv4 ip;
        rawIPv4 mask;
    };

    struct IPv6 {
        rawIPv6 ip;
        rawIPv6 mask;
    };
}

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

bool tryAccessUrl(const std::string& url, const char* httpHeader = nullptr);

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken);

void parseIPv4(const std::string& ip, NetTypes::IPv4& out);

void parseIPv6(const std::string& ip, NetTypes::IPv6& out);

bool resolveDomain(const std::string &hostname, std::set<std::string>& uniqueIPs);

#endif // NETWORK_HPP
