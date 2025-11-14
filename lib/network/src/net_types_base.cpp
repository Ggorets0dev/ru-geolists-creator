#include "net_types_base.hpp"
#include "exception.hpp"

#include <iostream>
#include <netdb.h>
#include <regex>

template <>
std::string NetTypes::IPvx<NetTypes::bitsetIPv4>::to_string() const {
    auto v = static_cast<uint32_t>(ip.to_ullong());
    int len = 0;
    auto m = static_cast<uint32_t>(mask.to_ullong());
    while (m) { len++; m <<= 1; }
    len = 32 - len;

    std::ostringstream oss;
    oss << ((v >> 24) & 0xFF) << '.'
        << ((v >> 16) & 0xFF) << '.'
        << ((v >>  8) & 0xFF) << '.'
        << (v & 0xFF) << '/' << len;
    return oss.str();
}

template <>
std::string NetTypes::IPvx<NetTypes::bitsetIPv6>::to_string() const {
    std::ostringstream oss;
    bool started = false;
    auto hi = static_cast<uint64_t>(ip.to_ullong() >> 64);
    auto lo = static_cast<uint64_t>(ip.to_ullong());

    auto print_part = [&](uint16_t p) {
        if (started || p != 0) {
            if (started) oss << ':';
            oss << std::hex << p;
            started = true;
        }
    };

    // Simplified output
    for (int i = 7; i >= 0; --i) print_part((hi >> (i * 16)) & 0xFFFF);
    for (int i = 7; i >= 0; --i) print_part((lo >> (i * 16)) & 0xFFFF);

    int len = 0;
    auto m = mask.to_ullong();
    while (m) { len++; m <<= 1; }
    len = 128 - len;

    oss << '/' << std::dec << len;
    return oss.str();
}

// ===========================
