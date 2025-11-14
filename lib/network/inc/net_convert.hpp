#ifndef NET_CONVERT_HPP
#define NET_CONVERT_HPP

#include <arpa/inet.h>

#include "net_types_base.hpp"

namespace NetUtils::Convert {
    void parseIPv4(const std::string& ip, NetTypes::IPv4Subnet& out);

    void parseIPv6(const std::string& ip, NetTypes::IPv6Subnet& out);

    NetTypes::bitsetIPv4 inetv4ToBitset(const in_addr& a);

    NetTypes::bitsetIPv6 inetv6ToBitset(const in6_addr& a);

    NetTypes::bitsetIPv4 lengthv4ToBitset(int len);

    NetTypes::bitsetIPv6 lengthv6ToBitset(int len);
}

#endif //NET_CONVERT_HPP