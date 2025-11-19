#include <catch2/catch_all.hpp>

#include "net_convert.hpp"
#include "net_types_base.hpp"

static bool allLeadingBitsSet(const NetTypes::bitsetIPv4& b, int n) {
    for (int i = 0; i < n; i++)
        if (!b[IPV4_BITS_COUNT - 1 - i]) return false;
    for (int i = n; i < IPV4_BITS_COUNT; i++)
        if (b[IPV4_BITS_COUNT - 1 - i]) return false;
    return true;
}

static bool allLeadingBitsSetV6(const NetTypes::bitsetIPv6& b, int n) {
    for (int i = 0; i < n; i++)
        if (!b[IPV6_BITS_COUNT - 1 - i]) return false;
    for (int i = n; i < IPV4_BITS_COUNT; i++)
        if (b[IPV6_BITS_COUNT - 1 - i]) return false;
    return true;
}

TEST_CASE("lengthv4ToBitset produces correct mask", "[ipv4]") {
    SECTION("len = 0") {
        auto const b = NetUtils::Convert::lengthv4ToBitset(0);
        REQUIRE(b.to_ulong() == 0);
    }

    SECTION("len = 1") {
        auto const b = NetUtils::Convert::lengthv4ToBitset(1);
        REQUIRE(allLeadingBitsSet(b, 1));
    }

    SECTION("len = 16") {
        auto const b = NetUtils::Convert::lengthv4ToBitset(16);
        REQUIRE(allLeadingBitsSet(b, 16));
    }

    SECTION("len = 32") {
        auto const b = NetUtils::Convert::lengthv4ToBitset(32);
        REQUIRE(b.to_ulong() == 0xFFFFFFFFu);
    }
}

TEST_CASE("lengthv6ToBitset produces correct mask", "[ipv6]") {
    SECTION("len = 0") {
        auto const b = NetUtils::Convert::lengthv6ToBitset(0);
        REQUIRE(b.none());
    }

    SECTION("len = 1") {
        auto const b = NetUtils::Convert::lengthv6ToBitset(1);
        REQUIRE(allLeadingBitsSetV6(b, 1));
    }

    SECTION("len = 64") {
        auto const b = NetUtils::Convert::lengthv6ToBitset(64);
        REQUIRE(allLeadingBitsSetV6(b, 64));
    }

    SECTION("len = 128") {
        auto const b = NetUtils::Convert::lengthv6ToBitset(128);
        REQUIRE(b.all());
    }
}