#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <variant>
#include <string>
#include <bitset>
#include <forward_list>

#define IPV4_BITS_COUNT         32u
#define IPV6_BITS_COUNT         128u

#define IPV6_HEX_GROUPS_COUNT   8u

namespace NetTypes {
    using bitsetIPv4 = std::bitset<IPV4_BITS_COUNT>;
    using bitsetIPv6 = std::bitset<IPV6_BITS_COUNT>;

    template <typename T>
    class IPvx {
    public:
        T ip;
        T mask;

        [[nodiscard]]
        bool isCorrupted() const {
            return ip.none() || mask.none();
        };

        bool isSubnetIncludes(const IPvx<T>& ipvx) const {
            const bool isAnyCorrupted = this->isCorrupted() || ipvx.isCorrupted();
            const bool isEqual = (this->ip & this->mask) == (ipvx.ip & ipvx.mask);

            return isEqual && !isAnyCorrupted;
        };

        [[nodiscard]]
        std::string to_string() const;
    };

    using IPv4Subnet = IPvx<bitsetIPv4>;
    using IPv6Subnet = IPvx<bitsetIPv6>;
    using SubnetVariant = std::variant<IPv4Subnet, IPv6Subnet>;

    template <typename T>
    using ListIPvx = std::forward_list<IPvx<T>>;

    using ListIPv4 = std::forward_list<IPv4Subnet>;
    using ListIPv6 = std::forward_list<IPv6Subnet>;

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

#endif // NETWORK_HPP
