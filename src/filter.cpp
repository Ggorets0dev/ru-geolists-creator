#include "filter.hpp"
#include "log.hpp"
#include <set>
#include <fstream>
#include <string>

#define FILTER_FILENAME_POSTFIX     "temp_filter"

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
        ipv4.push_front(bufferIPv4);
    } else if (type == NetTypes::AddressType::IPV6) {
        parseIPv6(buffer, bufferIPv6);
        ipv6.push_front(bufferIPv6);
    } else { // NetTypes::AddressType::DOMAIN
        std::forward_list<std::string> uniqueIPs;

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

void parseAddressFile(const fs::path& path, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6) {
    std::ifstream file(path);
    std::string buffer;
    bool status;

    size_t ipv4Size;
    size_t ipv6Size;

    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path.string());
    }

    while (std::getline(file, buffer)) {
        status = parseAddress(buffer, ipv4, ipv6);

        if (!status) {
            LOG_WARNING("An unknown entry was found in file with addresses, the type could not be determined: " + buffer);
        }
    }

    ipv4Size = std::distance(ipv4.begin(), ipv4.end());
    ipv6Size = std::distance(ipv6.begin(), ipv6.end());

    LOG_INFO("File " + path.string() + " parsed to " + std::to_string(ipv4Size) + " IPv4 entities and " + std::to_string(ipv6Size) + " IPv6 entities");
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
            status = listIP.isSubnetIncludes(checkIP);

            if (status) {
                return true;
            }
        }
    }

    // Check all IPv6
    for (const auto& checkIP : currIPv6) {
        for (const auto& listIP : ipv6) {
            status = listIP.isSubnetIncludes(checkIP);

            if (status) {
                return true;
            }
        }
    }

    return false;
}

bool checkFileByIPvLists(const fs::path& path, const NetTypes::ListIPv4& ipv4, const NetTypes::ListIPv6& ipv6, bool applyFix) {
    const fs::path tempFilePath = addPathPostfix(path, FILTER_FILENAME_POSTFIX);

    std::ifstream file;
    std::ofstream fileTemp;
    std::string buffer;
    bool status;
    bool isFoundAny = false;

    file.open(path);

    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path.string());
    }

    if (applyFix) {
        fileTemp.open(tempFilePath);

        if (!fileTemp.is_open()) {
            throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + tempFilePath.string());
        }
    }

    while (std::getline(file, buffer)) {
        status = checkAddressByLists(buffer, ipv4, ipv6);

        if (!status) {
            fileTemp << buffer;
        } else {
            isFoundAny = true;
            LOG_INFO("Detection in search between file and IP lists: " + buffer + " --> " + path.string());
        }
    }

    if (file.is_open()) {
        file.close();
    }

    if (applyFix && fileTemp.is_open()) {
        fileTemp.close();

        fs::remove(path);
        fs::rename(tempFilePath, path);
    }

    return isFoundAny;
}
