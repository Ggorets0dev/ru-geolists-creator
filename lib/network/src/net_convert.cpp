#include "net_convert.hpp"
#include "libnetwork_settings.hpp"
#include "bgp_parse.hpp"

#include <cstdint>

#include "log.hpp"

#define IPV6_PARTS_COUNT                8u

using namespace NetTypes;
using namespace NetUtils;

template <typename T>
static void parseSubnetIP(const std::string& ip, T& outIPVx) {
    int buffer;
    const auto pos = ip.find('/');

    outIPVx.mask.set();

    if (pos != std::string::npos) {
        // Subnet is specified
        buffer = std::stoi(ip.substr(pos + 1, 2));

        for (int i(0); i < buffer; ++i) {
            outIPVx.mask.reset(i);
        }
    } else if (gLibNetworkSettings.isSearchSubnetByBGP) {
        auto ptrie = BGP::getTrieFromCache();

        if (ptrie->isEmpty()) {
            if (gLibNetworkSettings.bgpDumpPath.empty()) {
                LOG_WARNING("Subnet cant be found using BGP dump because of incorrect dump path");
                return;
            }

            BGP::parseDumpToCache(gLibNetworkSettings.bgpDumpPath);
        }

        if constexpr (std::is_same_v<T, IPv4Subnet>) {
            if (auto ipb = ptrie->v4.lookup(outIPVx.ip)) {
                outIPVx.mask = ipb->mask;
            }

        } else if (std::is_same_v<T, IPv6Subnet>) {
            if (auto ipb = ptrie->v6.lookup(outIPVx.ip)) {
                outIPVx.mask = ipb->mask;
            }
        }
    }
}

bitsetIPv4 Convert::inetv4ToBitset(const in_addr& a) {
    uint32_t v = ntohl(a.s_addr);
    return {v};
}

bitsetIPv6 Convert::inetv6ToBitset(const in6_addr& a) {
    bitsetIPv6 b;
    for (int i = 0; i < 16; i++) {
        const uint8_t byte = a.s6_addr[i];
        for (int bit = 0; bit < 8; ++bit) {
            b[(15 - i) * 8 + (7 - bit)] = (byte >> bit) & 1;
        }
    }
    return b;
}

bitsetIPv4 Convert::lengthv4ToBitset(const int len) {
    uint32_t m = (len == 0) ? 0 : (0xFFFFFFFFu << (32 - len));
    return {m};
}

bitsetIPv6 Convert::lengthv6ToBitset(const int len) {
    bitsetIPv6 m;
    for (int i = 0; i < len; i++) {
        m[127 - i] = true;
    }
    return m;
}

void Convert::parseIPv4(const std::string& ip, IPv4Subnet& out) {
    uint8_t buffer;
    size_t pos;
    size_t start_pos(0);
    int8_t part_offset(24);

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

    parseSubnetIP(ip, out);
}

void Convert::parseIPv6(const std::string& ip, IPv6Subnet& out) {
    uint16_t buffer[IPV6_PARTS_COUNT] = {0};

    size_t pos;
    size_t start_pos(0);

    uint8_t i(0), j(0);
    uint8_t zero_secs_count(IPV6_PARTS_COUNT);
    uint8_t part_offset((IPV6_PARTS_COUNT - 1) * 16);

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

    parseSubnetIP(ip, out);
}