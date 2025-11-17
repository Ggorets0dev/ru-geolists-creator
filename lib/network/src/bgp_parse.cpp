#include <variant>

#include "bgp_parse.hpp"
#include "net_convert.hpp"
#include "bgpdump_lib.h"
#include "log.hpp"

using namespace NetTypes;
using namespace NetUtils;

static TriePair gCacheTrie;

static std::optional<SubnetVariant> extractSubnetFromEntry(BGPDUMP_ENTRY *entry);

void BGP::parseDump(const std::string& path, TriePair& outPair) {
    BGPDUMP* dump = nullptr;
    BGPDUMP_ENTRY* entry = nullptr;

    dump = bgpdump_open_dump(path.c_str());

    if (dump == nullptr) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path);
    }

    do {
        entry = bgpdump_read_next(dump);

        if (entry != nullptr) {
            auto subnet = extractSubnetFromEntry(entry);

            if (!subnet) {
                return;
            }

            std::visit([&outPair](auto &net){
                using T = std::decay_t<decltype(net)>;

                if constexpr (std::is_same_v<T, IPv4Subnet>) {
                    outPair.v4.insert(net);
                }
                if constexpr (std::is_same_v<T, IPv6Subnet>) {
                    outPair.v6.insert(net);
                }
            }, *subnet);

            bgpdump_free_mem(entry);
        }
    } while (dump->eof == 0);

    bgpdump_close_dump(dump);
}

void BGP::parseDumpToCache(const std::string& path) {
    parseDump(path, gCacheTrie);
}

const TriePair* BGP::getTrieFromCache() {
    return &gCacheTrie;
}

static std::optional<SubnetVariant> extractSubnetFromEntry(BGPDUMP_ENTRY *entry)
{
    if (!entry) {
        return std::nullopt;
    }

    // ===========================
    // TYPE: MRTD_TABLE_DUMP
    // ===========================
    if (entry->type == BGPDUMP_TYPE_MRTD_TABLE_DUMP) {

        if (entry->subtype == AFI_IP) {
            const auto ip  = Convert::inetv4ToBitset(entry->body.mrtd_table_dump.prefix.v4_addr);
            const auto mask = Convert::lengthv4ToBitset(entry->body.mrtd_table_dump.mask);

            return IPv4Subnet{ip, mask};
        }

#ifdef BGPDUMP_HAVE_IPV6
        if (entry->subtype == AFI_IP6) {
            const auto ip  = Convert::inetv6ToBitset(entry->body.mrtd_table_dump.prefix.v6_addr);
            const auto mask = Convert::lengthv6ToBitset(entry->body.mrtd_table_dump.mask);

            return IPv6Subnet{ip, mask};
        }
#endif
    }




    // ===========================
    // TYPE: TABLE_DUMP_V2
    // ===========================
    if (entry->type == BGPDUMP_TYPE_TABLE_DUMP_V2) {
        auto *p = &entry->body.mrtd_table_dump_v2_prefix;

        if (p->afi == AFI_IP) {
            const auto ip  = Convert::inetv4ToBitset(p->prefix.v4_addr);
            const auto mask = Convert::lengthv4ToBitset(p->prefix_length);

            return IPv4Subnet{ip, mask};
        }

#ifdef BGPDUMP_HAVE_IPV6
        if (p->afi == AFI_IP6) {
            const auto ip  = Convert::inetv6ToBitset(p->prefix.v6_addr);
            const auto mask = Convert::lengthv6ToBitset(p->prefix_length);

            return IPv6Subnet{ip, mask};
        }
#endif
    }




    // ===========================
    // TYPE: ZEBRA_BGP (берём source IP)
    // ===========================
    if (entry->type == BGPDUMP_TYPE_ZEBRA_BGP) {
        auto &m = entry->body.zebra_message;

        if (m.address_family == AFI_IP) {
            // Zebra не даёт маску — считаем /32
            const bitsetIPv4 ip = Convert::inetv4ToBitset(m.source_ip.v4_addr);
            const bitsetIPv4 mask = Convert::lengthv4ToBitset(IPV4_BITS_COUNT);

            return IPv4Subnet{ip, mask};
        }

#ifdef BGPDUMP_HAVE_IPV6
        if (m.address_family == AFI_IP6) {
            const bitsetIPv6 ip = Convert::inetv6ToBitset(m.source_ip.v6_addr);
            const bitsetIPv6 mask = Convert::lengthv6ToBitset(IPV6_BITS_COUNT);

            return IPv6Subnet{ip, mask};
        }
#endif
    }

    // Ничего не нашли
    return std::nullopt;
}