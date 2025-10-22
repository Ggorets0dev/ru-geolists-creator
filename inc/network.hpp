#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <algorithm>
#include <string>
#include <curl/curl.h>
#include <bitset>
#include <forward_list>
#include <future>
#include <ares.h>

#include "json_io.hpp"

// ====================
// Settings for C-Ares resolving
// ====================
#define RESOLVE_BATCH_SIZE              300u
// ====================

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
    };

    template <typename T>
    using ListIPvx = std::forward_list<NetTypes::IPvx<T>>;

    using ListIPv4 = std::forward_list<NetTypes::IPvx<NetTypes::bitsetIPv4>>;
    using ListIPv6 = std::forward_list<NetTypes::IPvx<NetTypes::bitsetIPv6>>;

    struct ListIPvxPair {
        ListIPv4& v4;
        ListIPv6& v6;
    };

    enum class AddressType {
        DOMAIN,
        IPV4,
        IPV6,
        UNKNOWN
    };

    using ListAddress = std::forward_list<std::string>;
}

namespace NetUtils {

    // Здесь должны быть твои типы и коллбэк
    // void CaresResolveCallback(...)

    class CAresResolver {
    public:
        struct ResolveQueryData {
            std::string host;
            std::promise<NetTypes::ListAddress> promise;
        };

        CAresResolver(unsigned int timeoutMs = 2000)
            : m_timeoutMs(timeoutMs), m_initialized(false)
        {
            m_initialized = init();
        }

        ~CAresResolver() {
            cleanup();
        }

        bool isInitialized() const {
            return m_initialized;
        }

        bool resolveDomains(const NetTypes::ListAddress& hosts, NetTypes::ListAddress& uniqueIPs);

    private:
        ares_channel m_channel{};
        unsigned int m_timeoutMs;
        bool m_initialized;

        static void resolveCallback(void *arg, int status, int, struct ares_addrinfo *res);

        bool init();

        void cleanup();

        void runEventLoop(std::vector<std::future<std::forward_list<std::string>>>& futures);
    };
}

NetTypes::AddressType getAddressType(const std::string& input);

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames);

bool tryAccessUrl(const std::string& url, const char* httpHeader = nullptr);

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken);

void parseIPv4(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv4>& out);

void parseIPv6(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv6>& out);

#endif // NETWORK_HPP
