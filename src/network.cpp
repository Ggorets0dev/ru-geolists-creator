#include "network.hpp"
#include "exception.hpp"
#include "log.hpp"
#include "common.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ares.h>
#include <regex>
#include <future>

#define GITHUB_TOKEN_HEADER         "Authorization: Bearer "

#define USER_AGENT                  "ru-geolists-creator"

#define CURL_OPERATION_TIMEOUT_SEC      10u
#define CURL_CONNECTION_TIMEOUT_SEC     5u

#define CONNECT_ATTEMPTS_COUNT          3u
#define CONNECT_ATTEMPT_DELAY_SEC       3u

#define DOWNLOAD_ATTEMPT_COUNT          3u
#define DOWNLOAD_ATTEMPT_DELAY_SEC      3u

#define IPV6_PARTS_COUNT                8u

struct ResolveQueryData {
    std::string host;
    std::promise<NetTypes::ListAddress> promise;
};

static void CaresResolveCallback(void *arg, int status, int, struct ares_addrinfo *res) {
    auto *qd = static_cast<ResolveQueryData*>(arg);
    NetTypes::ListAddress ips;
    if (status == ARES_SUCCESS) {
        char ipstr[INET6_ADDRSTRLEN];
        for (auto *node = res->nodes; node; node = node->ai_next) {
            void *addr = (node->ai_family == AF_INET)
            ? (void*)&((struct sockaddr_in*)node->ai_addr)->sin_addr
            : (void*)&((struct sockaddr_in6*)node->ai_addr)->sin6_addr;
            if (inet_ntop(node->ai_family, addr, ipstr, sizeof(ipstr)))
                ips.push_front(ipstr);
        }
        ares_freeaddrinfo(res);
    }

    qd->promise.set_value(std::move(ips));
}

static size_t writeToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;

    if (outFile->is_open()) {
        outFile->write(static_cast<char*>(contents), totalSize);
    }

    return totalSize;
}

static bool isUrlAccessible(const std::string& url, const char* httpHeader = nullptr) {
	CURL *curl = curl_easy_init();
    CURLcode res;
    volatile uint32_t responseCode(0);

    if (!curl) {
        curl_easy_cleanup(curl);
        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_OPERATION_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECTION_TIMEOUT_SEC);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

    if (httpHeader != nullptr && strlen(httpHeader) > 0) {
        struct curl_slist *header = nullptr;
        header = curl_slist_append(header, httpHeader);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    }

    res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    }

    curl_easy_cleanup(curl);

    return (res == CURLE_OK && responseCode >= 200 && responseCode < 400);
}

static void downloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr) {
    CURL* curl;
    CURLcode res;
    bool isAccessed;

    isAccessed = tryAccessUrl(url, httpHeader);

    if (!isAccessed) {
        throw CurlError("Failed to access URL in download handler", CURLE_COULDNT_CONNECT);
    }

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

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_OPERATION_TIMEOUT_SEC);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_CONNECTION_TIMEOUT_SEC);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

        if (httpHeader != nullptr && strlen(httpHeader) > 0) {
            struct curl_slist *header = nullptr;
            header = curl_slist_append(header, httpHeader);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        }

        // Perform query
        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        outFile.close();

        // Check for errors
        if (res != CURLE_OK) {
            throw CurlError("Failed to download file", res);
        }
    } else {
        curl_easy_cleanup(curl);
        outFile.close();

        throw CurlError("Failed to initialize cURL handle", CURLE_FAILED_INIT);
    }
}

static std::string genGithubTokenHeader(const std::string& token) {
    return GITHUB_TOKEN_HEADER + token;
}

bool tryAccessUrl(const std::string& url, const char* httpHeader) {
    for(uint8_t i(0); i < CONNECT_ATTEMPTS_COUNT; ++i) {
        if (isUrlAccessible(url, httpHeader)) {
            return true;
        } else {
            LOG_WARNING("Failed to access URL, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(CONNECT_ATTEMPT_DELAY_SEC));

            continue;
        }
    }

    return false;
}

template <typename T>
static void parseSubnetIP(const std::string& ip, T& mask) {
    int buffer;
    const auto pos = ip.find('/');

    mask.set();

    if (pos != std::string::npos) {
        // Subnet is specified
        buffer = std::stoi(ip.substr(pos + 1, 2));

        for (uint8_t i(0); i < buffer; ++i) {
            mask.reset(i);
        }
    }
}

bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken) {
    std::string tokenHeader = genGithubTokenHeader(apiToken);

    if (!apiToken.empty()) {
        return tryDownloadFile(url, filePath, tokenHeader.c_str());
    } else {
        return tryDownloadFile(url, filePath);
    }
}

bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader) {
    for(uint8_t i(0); i < DOWNLOAD_ATTEMPT_COUNT; ++i) {
        try {
            downloadFile(url, filePath, httpHeader);
        }  catch (std::exception& e) {
            LOG_ERROR(e.what());
            LOG_WARNING("Failed to download requested file, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(DOWNLOAD_ATTEMPT_DELAY_SEC));

            continue;
        }

        return true;
    }

    return false;
}

bool downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames) {
    bool status;

    if (value.isMember("assets") && value["assets"].isArray()) {
        const Json::Value& assets = value["assets"];
        for (const auto& asset : assets) {
            if (std::find(fileNames.begin(), fileNames.end(), asset["name"].asString()) == fileNames.end()) {
                continue;
            }

            status = tryDownloadFile(asset["browser_download_url"].asString(), asset["name"].asString());

            if (!status) {
                return status;
            }
        }
    }

    return true;
}

bool resolveDomains(const NetTypes::ListAddress& hosts, NetTypes::ListAddress& uniqueIPs) {
    ares_channel channel;
    std::vector<ResolveQueryData> queries;
    std::vector<std::future<std::forward_list<std::string>>> futures;

    const size_t hostsSize = std::distance(hosts.begin(), hosts.end());

    queries.reserve(hostsSize);
    futures.reserve(hostsSize);

    int status = ares_library_init(ARES_LIB_INIT_ALL);

    if (status != ARES_SUCCESS) {
        LOG_ERROR("Failed to init C-Ares library: " + std::string(ares_strerror(status)));
        return false;
    }

    struct ares_options options;
    int optmask = 0;

    options.timeout = 2000; //  ms // FIXME: TAKE FROM CONFIG OR MACRO
    optmask |= ARES_OPT_TIMEOUTMS;

    status = ares_init_options(&channel, &options, optmask);

    if (status != ARES_SUCCESS) {
        LOG_ERROR("Failed to init C-Ares control block: " + std::string(ares_strerror(status)));
        ares_library_cleanup();
        return false;
    }

    // Performing queries using C-Ares and adding futures for sync
    for (auto &h : hosts) {
        queries.push_back({h});
        futures.push_back(queries.back().promise.get_future());

        ares_addrinfo_hints hints{};
        hints.ai_family = AF_UNSPEC;
        ares_getaddrinfo(channel, h.c_str(), nullptr, &hints, CaresResolveCallback, &queries.back());
    }

    //=================

    // единый event loop
    bool done = false;
    while (!done) {
        fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        int socks[ARES_GETSOCK_MAXNUM];
        int bitmask = ares_getsock(channel, socks, ARES_GETSOCK_MAXNUM);
        int maxfd = -1;
        for (int i = 0; i < ARES_GETSOCK_MAXNUM; i++) {
            if (ARES_GETSOCK_READABLE(bitmask, i)) {
                FD_SET(socks[i], &rfds);
                if (socks[i] > maxfd) maxfd = socks[i];
            }
            if (ARES_GETSOCK_WRITABLE(bitmask, i)) {
                FD_SET(socks[i], &wfds);
                if (socks[i] > maxfd) maxfd = socks[i];
            }
        }

        timeval tv;
        auto tvp = ares_timeout(channel, nullptr, &tv);

        int nfds = (maxfd >= 0) ? maxfd + 1 : 0;
        int rc = select(nfds, &rfds, &wfds, nullptr, tvp);

        if (rc >= 0) {
            ares_process(channel, &rfds, &wfds);
        }

        // Check if all requests are performed
        done = std::all_of(futures.begin(), futures.end(),
                           [](auto &f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; });
    }

    for (auto &f : futures) {
        auto ips = f.get();
        for (auto &ip : ips) {
            uniqueIPs.push_front(ip);
        }
    }

    ares_destroy(channel);
    ares_library_cleanup();

    removeListDuplicates(uniqueIPs);

    return (std::distance(uniqueIPs.begin(), uniqueIPs.end()) != 0);
}


void parseIPv4(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv4>& out) {
    uint8_t buffer;
    size_t pos;
    size_t start_pos(0);
    int8_t part_offset(24);

    parseSubnetIP(ip, out.mask);

    out.ip.reset();

    do {
        if (part_offset) {
            pos = ip.find('.', start_pos);
        } else {
            pos = ip.find('/', start_pos);
        }

        if (!part_offset && pos == std::string::npos) {
            pos = std::distance(ip.begin() + start_pos, ip.end());
        }

        if (pos == std::string::npos) {
            // Throw exception
        }

        buffer = std::stoi(ip.substr(start_pos, pos - start_pos));

        out.ip |= (buffer << part_offset);

        part_offset -= 8;
        start_pos = pos + 1;
    } while (part_offset >= 0);
}

void parseIPv6(const std::string& ip, NetTypes::IPvx<NetTypes::bitsetIPv6>& out) {
    uint16_t buffer[IPV6_PARTS_COUNT] = {0};

    size_t pos;
    size_t start_pos(0);

    uint8_t i(0), j(0);
    uint8_t zero_secs_count(IPV6_PARTS_COUNT);
    uint8_t part_offset((IPV6_PARTS_COUNT - 1) * 16);

    parseSubnetIP(ip, out.mask);

    out.ip.reset();

    while (true) {
        pos = ip.find(':', start_pos);

        if (pos == std::string::npos) {
            pos = ip.find('/', start_pos);

            if (pos == std::string::npos) {
                break;
            }
        }

        auto substr = ip.substr(start_pos, pos - start_pos);

        if (substr.length() > 1) {
            buffer[i] = std::stoi(substr, nullptr, 16);
            --zero_secs_count;
        } else {
            buffer[i] = 0;
        }

        ++i;

        start_pos = pos + 1;
    };

    while (j < i) {
        if (buffer[j]) {
            out.ip |= (buffer[j] << part_offset);
            ++j;
        } else {
            --zero_secs_count;

            if (!zero_secs_count) {
                ++j;
            }
        }

        part_offset -= 16;
    }
}



NetTypes::AddressType getAddressType(const std::string& input) {
    try {
        // IPv4 with mask /0–32
        static const std::regex kPatternIPv4(
            R"(^(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.)"
            R"((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d))"
            R"((/(3[0-2]|[12]?\d))?$)"
        );

        // IPv6 (simplified, ECMAScript-compatible)
        static const std::regex kPatternIPv6(
            R"(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"
        );

        static const std::regex kPatternDomain(
            R"(^(?!-)([A-Za-z0-9-]{1,63}\.)+[A-Za-z]{2,63}$)"
        );

        if (std::regex_match(input, kPatternIPv4)) {
            return NetTypes::AddressType::IPV4;
        }
        if (std::regex_match(input, kPatternIPv6)) {
            return NetTypes::AddressType::IPV6;
        }
        if (std::regex_match(input, kPatternDomain)) {
            return NetTypes::AddressType::DOMAIN;
        }

        return NetTypes::AddressType::UNKNOWN;
    }
    catch (const std::regex_error& e) {
        LOG_ERROR("Regex error occured after trying to get address type: " << e.what());
        return NetTypes::AddressType::UNKNOWN;
    }
}
