#include "net_types_base.hpp"
#include "exception.hpp"

#include <netdb.h>
#include <regex>

template <>
std::string NetTypes::IPv4Subnet::to_string() const {
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
std::string NetTypes::IPv6Subnet::to_string() const {
    std::ostringstream oss;
    bool started = false;

    // Extract one 16-bit block (hextet) from the 128-bit IPv6 address
    auto get16 = [&](int block) -> uint16_t {  // block: 0 to 7
        uint16_t v = 0;
        int bit_offset = 127 - block * 16;  // starting bit: 127, 111, ..., 15
        for (int b = 0; b < 16; ++b) {
            int bit = bit_offset - b;
            if (ip.test(bit)) {
                v |= static_cast<uint16_t>(1) << (15 - b);
            }
        }
        return v;
    };

    auto print_part = [&](uint16_t p) {
        if (started) oss << ':';
        oss << std::hex << p;
        started = true;
    };

    // Only 8 hextets in IPv6
    for (int blk = 0; blk < 8; ++blk) {
        print_part(get16(blk));
    }

    // Count prefix length
    int prefix = 0;
    for (int i = 127; i >= 0; --i) {
        if (mask.test(i)) ++prefix;
        else break;
    }

    oss << '/' << std::dec << prefix;
    return oss.str();
}


// ===========================
