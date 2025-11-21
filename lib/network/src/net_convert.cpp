#include "net_convert.hpp"
#include "libnetwork_settings.hpp"
#include "net_types_base.hpp"
#include "bgp_parse.hpp"
#include "log.hpp"

#include <cstdint>

using namespace NetTypes;
using namespace NetUtils;

static constexpr bool validateOctetIPv4(const int value) {
    return (value >= 0 && value <= 255);
}

// TODO: VALIDATION FOR IPV6

template <typename T>
static bool parseSubnetIP(const std::string& ip, T& outIPVx) {
    int buffer;
    const auto pos = ip.find('/');
    const auto size = outIPVx.mask.size();
    bool searchStatus = true;
    bool filterStatus = true;

    outIPVx.mask.reset();

    if (pos != std::string::npos) {
        // Subnet is specified
        buffer = std::stoi(ip.substr(pos + 1, 2));

        for (int i(0); i < buffer; ++i) {
            outIPVx.mask.set(size - 1 - i);
        }
    } else if (gLibNetworkSettings.isSearchSubnetByBGP) {
        auto ptrie = BGP::getTrieFromCache();

        if (ptrie->isEmpty()) {
            if (gLibNetworkSettings.bgpDumpPath.empty()) {
                LOG_WARNING("Subnet cant be found using BGP dump because of incorrect dump path");
                return false;
            }

            BGP::parseDumpToCache(gLibNetworkSettings.bgpDumpPath);
        }

        if constexpr (std::is_same_v<T, IPv4Subnet>) {
            if (auto ipb = ptrie->v4.lookup(outIPVx.ip)) {
                outIPVx.mask = ipb->mask;
                filterStatus = Convert::bitsetToLength(ipb->mask) >= gLibNetworkSettings.autoFixMaskLimitByBGP.v4;
            } else {
                searchStatus = false;
            }
        } else if (std::is_same_v<T, IPv6Subnet>) {
            if (auto ipb = ptrie->v6.lookup(outIPVx.ip)) {
                outIPVx.mask = ipb->mask;
                filterStatus = Convert::bitsetToLength(ipb->mask) >= gLibNetworkSettings.autoFixMaskLimitByBGP.v6;
            } else {
                searchStatus = false;
            }
        }

        if (!searchStatus) {
            LOG_WARNING("Failed to find subnet for IP using BGP dump: " + outIPVx.to_string());
            return false;
        } else if (!filterStatus) {
            // TODO: Add log, but not in every tact
            return false;
        }
    }

    if (outIPVx.mask.count() == 0) {
        outIPVx.mask.set();
    }

    return searchStatus && filterStatus;
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

template <size_t N>
int Convert::bitsetToLength(const std::bitset<N>& bs) {
    int prefix = 0;

    for (int i = N - 1; i >= 0; --i) {
        if (!bs.test(i))
            break;
        ++prefix;
    }

    return prefix;
}

bool Convert::parseIPv4(const std::string& ip, IPv4Subnet& out) {
    int buffer;
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
            // Failed to find octet end
            return false;
        }

        buffer = std::stoi(ip.substr(start_pos, pos - start_pos));

        if (!validateOctetIPv4(buffer)) {
            // Value is out of bound in octet
            return false;
        }

        out.ip |= (buffer << part_offset);

        part_offset -= 8;
        start_pos = pos + 1;
    } while (part_offset >= 0);

    return parseSubnetIP(ip, out);
}

bool Convert::parseIPv6(const std::string& ip, IPv6Subnet& out) {
    // найдём часть до '/': parseSubnetIP сам распарсит префикс
    size_t slash_pos = ip.find('/');
    std::string addr = (slash_pos == std::string::npos) ? ip : ip.substr(0, slash_pos);

    out.ip.reset();

    // split helper (разделитель ':')
    auto split_by_colon = [](const std::string& s) {
        std::vector<std::string> res;
        size_t start = 0;
        size_t pos;
        while ((pos = s.find(':', start)) != std::string::npos) {
            res.push_back(s.substr(start, pos - start));
            start = pos + 1;
        }
        res.push_back(s.substr(start));
        return res;
    };

    std::vector<std::string> parts;

    size_t dbl_pos = addr.find("::");
    if (dbl_pos != std::string::npos) {
        // есть сокращение
        std::string left = addr.substr(0, dbl_pos);
        std::string right = addr.substr(dbl_pos + 2);

        std::vector<std::string> left_parts;
        std::vector<std::string> right_parts;
        if (!left.empty()) left_parts = split_by_colon(left);
        if (!right.empty()) right_parts = split_by_colon(right);

        // special case: "::" alone -> left_parts.empty() && right_parts.empty()
        size_t missing = 8;
        if (left_parts.size() + right_parts.size() <= 8) {
            missing = 8 - (left_parts.size() + right_parts.size());
        } else {
            return false; // слишком много групп
        }

        parts = left_parts;
        for (size_t i = 0; i < missing; ++i) parts.emplace_back("0");
        parts.insert(parts.end(), right_parts.begin(), right_parts.end());
    } else {
        parts = split_by_colon(addr);
    }

    if (parts.size() != 8) {
        // возможно IPv4-совместимый формат не поддерживаем здесь
        return false;
    }

    // Парсим каждую 16-битную группу и записываем в bitset
    // group_index = 0..7 (0 — самая левая группа), внутри группы j = 0..15 (старший бит — j=0)
    for (size_t group_index = 0; group_index < 8; ++group_index) {
        const std::string& grp = parts[group_index];
        if (grp.empty()) return false; // на всякий случай

        // допускаем 1..4 hex-символа
        if (grp.size() > 4) return false;

        unsigned long value = 0;
        try {
            value = std::stoul(grp, nullptr, 16);
        } catch (...) {
            return false;
        }
        if (value > 0xFFFFUL) return false;

        // для каждого бита внутри 16-битной группы:
        // j = 0..15 — это позиция внутри группы, где j=0 соответствует старшему биту группы (1<<15)
        for (int j = 0; j < 16; ++j) {
            // проверяем старший первый (1 << (15 - j))
            if (value & (1u << (15 - j))) {
                // позиция в 128-битовом числе (0..127), где 127 — самый старший бит всего адреса
                int ipv6_bit_index = static_cast<int>(group_index * 16 + j); // 0..127 (0 — старший бит группы0)
                // переводим в индекс std::bitset (где 0 — LSB)
                int bitset_index = 127 - ipv6_bit_index;
                out.ip.set(bitset_index);
            }
        }
    }

    return parseSubnetIP(ip, out);
}
