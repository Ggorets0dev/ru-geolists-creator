#include <catch2/catch_all.hpp>

#include "net_convert.hpp"
#include "net_types_base.hpp"
#include "url_handle.hpp"
#include "libnetwork_settings.hpp"

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

struct FastTimeout {
    FastTimeout() {
        orig_conn = gLibNetworkSettings.curlConnectionTimeoutSec;
        orig_op   = gLibNetworkSettings.curlOperationTimeoutSec;
        orig_conn_at = gLibNetworkSettings.connectAttemptsCount;

        gLibNetworkSettings.curlConnectionTimeoutSec = 2;
        gLibNetworkSettings.curlOperationTimeoutSec = 5;
        gLibNetworkSettings.connectAttemptsCount = 1;
    }
    ~FastTimeout() {
        gLibNetworkSettings.curlConnectionTimeoutSec = orig_conn;
        gLibNetworkSettings.curlOperationTimeoutSec = orig_op;
        gLibNetworkSettings.connectAttemptsCount = orig_conn_at;
    }
    long orig_conn = 0, orig_op = 0, orig_conn_at = 0;
};

TEST_CASE("tryAccessUrl: happy path — 200 OK + redirect chain", "[url]") {
    FastTimeout ft;

    REQUIRE(NetUtils::tryAccessUrl("https://httpbin.org/status/200") == true);
    REQUIRE(NetUtils::tryAccessUrl("https://httpbin.org/redirect/3") == true);  // проверка FOLLOWLOCATION
}

TEST_CASE("tryAccessUrl: error responses — 4xx и 5xx", "[url]") {
    FastTimeout ft;

    REQUIRE(NetUtils::tryAccessUrl("https://httpbin.org/status/404") == false);
    REQUIRE(NetUtils::tryAccessUrl("https://httpbin.org/status/500") == false);
}

TEST_CASE("tryAccessUrl: network/DNS/SSL failures", "[url]") {
    FastTimeout ft;

    REQUIRE(NetUtils::tryAccessUrl("https://this-domain-should-never-exist-42-42-42.com") == false);  // DNS
    REQUIRE(NetUtils::tryAccessUrl("https://expired.badssl.com") == false);                       // SSL error
    REQUIRE(NetUtils::tryAccessUrl("http://10.255.255.1") == false);                               // connection refused
}

TEST_CASE("tryAccessUrl: malformed URLs + custom header", "[url]") {
    FastTimeout ft;

    REQUIRE(NetUtils::tryAccessUrl("") == false);
    REQUIRE(NetUtils::tryAccessUrl("not-a-url") == false);
    REQUIRE(NetUtils::tryAccessUrl("https://httpbin.org/headers", "X-Test: yes") == true);  // header работает
}

TEST_CASE("getAddressType: valid IPv4 addresses", "[ipv4][valid]") {
    SECTION("basic IPv4") {
        REQUIRE(NetUtils::getAddressType("192.168.1.1")     == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("8.8.8.8")         == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("0.0.0.0")         == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("255.255.255.255") == NetTypes::AddressType::IPV4);
    }

    SECTION("IPv4 with CIDR mask") {
        REQUIRE(NetUtils::getAddressType("10.0.0.0/8")      == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("172.16.0.0/12")   == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("192.168.100.1/24")== NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("1.1.1.1/0")       == NetTypes::AddressType::IPV4);
        REQUIRE(NetUtils::getAddressType("255.255.255.255/32") == NetTypes::AddressType::IPV4);
    }
}

TEST_CASE("getAddressType: invalid IPv4", "[ipv4][invalid]") {
    REQUIRE(NetUtils::getAddressType("256.256.256.256") == NetTypes::AddressType::UNKNOWN);   // > 255
    REQUIRE(NetUtils::getAddressType("1.2.3")           == NetTypes::AddressType::UNKNOWN);   // не 4 октета
    REQUIRE(NetUtils::getAddressType("1.2.3.4.5")       == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("192.168.1.999")   == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("192.168.01.1")    == NetTypes::AddressType::UNKNOWN);   // leading zero не запрещён в твоём regex, но если хочешь — добавь проверку
    REQUIRE(NetUtils::getAddressType("192.168.1.1/33")  == NetTypes::AddressType::UNKNOWN);   // маска > 32
    REQUIRE(NetUtils::getAddressType("192.168.1.1/-1")  == NetTypes::AddressType::UNKNOWN);   // отрицательная маска
    REQUIRE(NetUtils::getAddressType("192.168.1.1/")    == NetTypes::AddressType::UNKNOWN);   // только слеш
}

TEST_CASE("getAddressType: valid IPv6 addresses", "[ipv6][valid]") {
    SECTION("full form") {
        REQUIRE(NetUtils::getAddressType("2001:0db8:85a3:0000:0000:8a2e:0370:7334") ==NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("fe80::1")                                 == NetTypes::AddressType::IPV6);
    }

    SECTION("compressed form") {
        REQUIRE(NetUtils::getAddressType("2001:db8::")          == NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("::1")                 == NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("2001:db8::8a2e:370:7334") == NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("fe80::")              == NetTypes::AddressType::IPV6);
    }

    SECTION("IPv4-mapped IPv6") {
        REQUIRE(NetUtils::getAddressType("::ffff:192.168.1.1")  == NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("::ffff:8.8.8.8")      == NetTypes::AddressType::IPV6);
    }

    SECTION("link-local with zone") {
        REQUIRE(NetUtils::getAddressType("fe80::1%eth0")        == NetTypes::AddressType::IPV6);
        REQUIRE(NetUtils::getAddressType("fe80::abcd%enp0s3")   == NetTypes::AddressType::IPV6);
    }
}

TEST_CASE("getAddressType: invalid IPv6", "[ipv6][invalid]") {
    REQUIRE(NetUtils::getAddressType("2001:db8::g")          == NetTypes::AddressType::UNKNOWN); // буква g
    REQUIRE(NetUtils::getAddressType("2001:db8::::")         == NetTypes::AddressType::UNKNOWN); // два ::
    REQUIRE(NetUtils::getAddressType("2001:db8:85a3:0000:0000:8a2e:0370:7334:1234") == NetTypes::AddressType::UNKNOWN); // слишком много групп
    REQUIRE(NetUtils::getAddressType("fe80::1%")             == NetTypes::AddressType::UNKNOWN); // пустой zone
    REQUIRE(NetUtils::getAddressType("g::1")                 == NetTypes::AddressType::UNKNOWN);
}

TEST_CASE("getAddressType: valid domain names", "[domain][valid]") {
    REQUIRE(NetUtils::getAddressType("example.com")              == NetTypes::AddressType::DOMAIN);
    REQUIRE(NetUtils::getAddressType("sub.domain.co.uk")         == NetTypes::AddressType::DOMAIN);
    REQUIRE(NetUtils::getAddressType("x.com")                      == NetTypes::AddressType::DOMAIN); // минимальный валидный
    REQUIRE(NetUtils::getAddressType("xn--80asehdb.com")         == NetTypes::AddressType::DOMAIN); // punycode
    REQUIRE(NetUtils::getAddressType("very-very-very-long-domain-name-that-is-still-valid.bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb.example") == NetTypes::AddressType::DOMAIN);
    REQUIRE(NetUtils::getAddressType("GOOGLE.COM")               == NetTypes::AddressType::DOMAIN); // uppercase
    REQUIRE(NetUtils::getAddressType("localhost")                == NetTypes::AddressType::DOMAIN);
}

TEST_CASE("getAddressType: invalid domain names", "[domain][invalid]") {
    REQUIRE(NetUtils::getAddressType("-example.com")             == NetTypes::AddressType::UNKNOWN); // начинается с дефиса
    REQUIRE(NetUtils::getAddressType("example-.com")             == NetTypes::AddressType::UNKNOWN); // заканчивается дефисом
    REQUIRE(NetUtils::getAddressType("example.com.")             == NetTypes::AddressType::UNKNOWN); // trailing dot (твой regex его не пропускает)
    REQUIRE(NetUtils::getAddressType(".example.com")             == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("ex..ample.com")            == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("toolonglabel1234567890123456789012345678901234567890123456789012345678901234567890.com") == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("example.c")                == NetTypes::AddressType::UNKNOWN); // TLD слишком короткий
}

TEST_CASE("getAddressType: edge cases and mixed", "[edge]") {
    // Приоритет: IPv4 > IPv6 > Domain
    REQUIRE(NetUtils::getAddressType("192.168.1.1")      == NetTypes::AddressType::IPV4);     // IPv4 важнее домена
    REQUIRE(NetUtils::getAddressType("2001:db8::")       == NetTypes::AddressType::IPV6);
    REQUIRE(NetUtils::getAddressType("example.com")      == NetTypes::AddressType::DOMAIN);

    // Похоже на IP, но не валидно → UNKNOWN
    REQUIRE(NetUtils::getAddressType("999.999.999.999") == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("::ffff:999.999.999.999") == NetTypes::AddressType::UNKNOWN);

    // Пустая строка и мусор
    REQUIRE(NetUtils::getAddressType("")                 == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("   ")              == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("hello world")      == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("user@host.com")    == NetTypes::AddressType::UNKNOWN);
    REQUIRE(NetUtils::getAddressType("http://example.com") == NetTypes::AddressType::UNKNOWN);
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