#include "filter.hpp"
#include "log.hpp"
#include <set>
#include <fstream>
#include <string>

static bool parseAddress(const std::string& buffer, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6) {
    NetTypes::AddressType type;
    NetTypes::IPvx<NetTypes::bitsetIPv4> bufferIPv4;
    NetTypes::IPvx<NetTypes::bitsetIPv6> bufferIPv6;
    bool status;

    type = getAddressType(buffer);

    if (type == NetTypes::AddressType::UNKNOWN) {
        return false;
    }

    if (type == NetTypes::AddressType::IPV4) {
        parseIPv4(buffer, bufferIPv4);
        ipv4.insert(bufferIPv4);
    } else if (type == NetTypes::AddressType::IPV6) {
        parseIPv6(buffer, bufferIPv6);
        ipv6.insert(bufferIPv6);
    } else { // NetTypes::AddressType::DOMAIN
        std::set<std::string> uniqueIPs;

        status = resolveDomain(buffer, uniqueIPs);

        if (!status) {
            // Resolve failed, nothing to parse
            return false;
        }

        for (const auto& uip : uniqueIPs) {
            status &= parseAddress(uip, ipv4, ipv6);
        }

        return status;
    }

    return true;
}

static void parseAddressFile(const fs::path& path, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6) {
    std::ifstream file(path);
    std::string buffer;
    bool status;

    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path.string());
    }

    while (std::getline(file, buffer)) {
        status = parseAddress(buffer, ipv4, ipv6);

        if (!status) {
            LOG_WARNING("An unknown entry was found in file with addresses, the type could not be determined: " + buffer);
        }
    }
}

bool checkAddressByLists(const std::string& addr, const NetTypes::ListIPv4& ipv4, const NetTypes::ListIPv6& ipv6) {
    bool status;

    static NetTypes::ListIPv4 currIPv4;
    static NetTypes::ListIPv6 currIPv6;

    currIPv4.clear();
    currIPv6.clear();

    status = parseAddress(addr, currIPv4, currIPv6);

    if (!status) {
        return false;
    }

    // Check all IPv4
    for (const auto& checkIP : currIPv4) {
        for (const auto& listIP : ipv4) {
            status = !listIP.isSubnetIncludes(checkIP);

            if (status) {
                return true;
            }
        }
    }

    // Check all IPv6
    for (const auto& checkIP : currIPv6) {
        for (const auto& listIP : ipv6) {
            status = !listIP.isSubnetIncludes(checkIP);

            if (status) {
                return true;
            }
        }
    }

    return true;
}

bool checkFileByLists(const fs::path& path, const NetTypes::ListIPv4& ipv4, const NetTypes::ListIPv6& ipv6) {
    std::ifstream file(path);
    std::string buffer;
    bool status;
    bool isFoundAny = false;

    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path.string());
    }

    while (std::getline(file, buffer)) {
        status = checkAddressByLists(buffer, ipv4, ipv6);

        if (!status) {
            isFoundAny = true;
            LOG_INFO("Detection in search between file and IP lists: " + buffer + " --> " + path.string());
        }
    }

    return isFoundAny;
}
