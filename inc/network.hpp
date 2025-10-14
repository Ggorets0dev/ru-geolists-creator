#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <curl/curl.h>
#include <set>
#include <bitset>

#include "json_io.hpp"

namespace NetTypes {
    using bitsetIPv4 = std::bitset<32>;
    using bitsetIPv6 = std::bitset<128>;

    template <typename T>
    class IPvx {
    public:
        T ip;
        T mask;

        bool isSubnetIncludes(const IPvx<T>& ipvx) const {
            return (this->ip & this->mask) == (ipvx.ip & ipvx.mask);
        };

        bool operator<(const IPvx<T>& other) const {
            // Placeholder logic, just need for std::set to work. No real sort needed
            return this->ip == other.ip;
        }
    };

    using ListIPv4 = std::set<NetTypes::IPvx<NetTypes::bitsetIPv4>>;
    using ListIPv6 = std::set<NetTypes::IPvx<NetTypes::bitsetIPv6>>;

    enum class AddressType {
        DOMAIN,
        IPV4,
        IPV6,
        UNKNOWN
    };
}

NetTypes::AddressType getAddressType(const std::string& input);

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

bool tryAccessUrl(const std::string& url, const char* httpHeader = nullptr);

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken);

void parseIPv4(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv4>& out);

void parseIPv6(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv6>& out);

bool resolveDomain(const std::string &hostname, std::set<std::string>& uniqueIPs);

#endif // NETWORK_HPP
