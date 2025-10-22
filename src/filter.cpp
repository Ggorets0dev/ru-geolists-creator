#include "filter.hpp"
#include "log.hpp"
#include "common.hpp"
#include <set>
#include <fstream>
#include <iostream>
#include <string>

#define FILTER_FILENAME_POSTFIX     "temp_filter"

static bool parseAddress(const std::string& buffer, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6, NetTypes::ListAddress* domainBuffer=nullptr) {
    NetTypes::AddressType type;
    NetTypes::IPvx<NetTypes::bitsetIPv4> bufferIPv4;
    NetTypes::IPvx<NetTypes::bitsetIPv6> bufferIPv6;
    bool status;

    type = getAddressType(buffer);

    if (type == NetTypes::AddressType::IPV4) {
        parseIPv4(buffer, bufferIPv4);
        ipv4.push_front(bufferIPv4);
    } else if (type == NetTypes::AddressType::IPV6) {
        parseIPv6(buffer, bufferIPv6);
        ipv6.push_front(bufferIPv6);
    } else if (type == NetTypes::AddressType::DOMAIN && (domainBuffer != nullptr)) {
        domainBuffer->push_front(buffer);
    } else if (type == NetTypes::AddressType::DOMAIN && (domainBuffer == nullptr)) {
        // Nothing to do, skipping
    } else { // NetTypes::AddressType::UNKNOWN
        // Failed to get address type
        return false;
    }

    return true;
}

static bool parseAddress(const NetTypes::ListAddress& addrs, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6) {
    bool status = true;

    for (const auto& addr : addrs) {
        status &= parseAddress(addr, ipv4, ipv6);
    }

    return status;
}

void parseAddressFile(const fs::path& path, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6) {
    std::ifstream file(path);
    std::string buffer;
    bool status;

    NetTypes::ListAddress domainsBuffer;
    NetTypes::ListAddress uniqueIPs;

    size_t ipv4Size;
    size_t ipv6Size;

    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + path.string());
    }

    while (std::getline(file, buffer)) {
        status = parseAddress(buffer, ipv4, ipv6, &domainsBuffer);

        if (!status) {
            LOG_WARNING("An unknown entry was found in file with addresses, the type could not be determined: " + buffer);
        }
    }

    // ======== Convert all domains to IPv4 or IPv6
    NetUtils::CAresResolver resolver(2000);

    if (!resolver.isInitialized()) {
        std::cerr << "Failed to init resolver\n";
        return;
    }

    resolver.resolveDomains(domainsBuffer, uniqueIPs);

    domainsBuffer.clear();

    for (const auto& IP : uniqueIPs) {
        status = parseAddress(IP, ipv4, ipv6);
    }
    // ========

    ipv4Size = std::distance(ipv4.begin(), ipv4.end());
    ipv6Size = std::distance(ipv6.begin(), ipv6.end());

    LOG_INFO("File " + path.string() + " parsed to " + std::to_string(ipv4Size) + " IPv4 entities and " + std::to_string(ipv6Size) + " IPv6 entities");
}

template <typename T>
static bool checkIPvxByLists(NetTypes::ListIPvx<T>& current, const NetTypes::ListIPvx<T>& lists, std::forward_list<uint32_t>* removeIndices=nullptr) {
    bool checkResult = false;
    size_t index = 0;

    for (const auto& cIP : current) {
        for (const auto& lIP : lists) {
            bool status = lIP.isSubnetIncludes(cIP);

            if (status && (removeIndices == nullptr)) {
                return true;
            } else if (status) {
                checkResult = true;
                removeIndices->push_front(index);

                break;
            }
        }
        ++index;
    }

    return checkResult;
}


bool checkFileByIPvLists(const fs::path& path, const NetTypes::ListIPvxPair& listsPair, bool applyFix) {
    const fs::path tempFilePath = addPathPostfix(path, FILTER_FILENAME_POSTFIX);

    NetTypes::ListIPv4 currIPv4;
    NetTypes::ListIPv6 currIPv6;
    NetTypes::ListAddress uniqueIPs;

    std::ifstream file;
    std::ofstream fileTemp;

    std::string buffer;
    NetTypes::ListAddress domainBatch;

    std::forward_list<uint32_t> removeIndicies;

    bool status;
    bool isFoundAny(false);

    float progress;
    const size_t linesCount = countLinesInFile(path);
    size_t currSize(0);
    size_t currPerfCount(0);

    NetUtils::CAresResolver resolver(2000);

    if (!resolver.isInitialized()) {
        std::cerr << "Failed to init resolver\n";
        return 1;
    }

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
        auto type = getAddressType(buffer);
        bool isTypeDomain = type == NetTypes::AddressType::DOMAIN;

        status = false;

        removeIndicies.clear();

        if (type == NetTypes::AddressType::UNKNOWN) {
            LOG_WARNING("An unknown entry was found in file with addresses, the type could not be determined: " + buffer);
            continue;
        }

        if (isTypeDomain) {
            domainBatch.push_front(buffer);
            ++currSize;
        }

        if (isTypeDomain && (currSize == RESOLVE_BATCH_SIZE || currPerfCount == linesCount - 1)) {
            resolver.resolveDomains(domainBatch, uniqueIPs);
            parseAddress(uniqueIPs, currIPv4, currIPv6);

            status |= checkIPvxByLists(currIPv4, listsPair.v4, &removeIndicies);
            status |= checkIPvxByLists(currIPv6, listsPair.v6, &removeIndicies);

            currPerfCount += currSize;
        } else if (isTypeDomain) {
            // Not enough recording in batch, skipping
            continue;
        } else {
            parseAddress(buffer, currIPv4, currIPv6);

            status |= checkIPvxByLists(currIPv4, listsPair.v4);
            status |= checkIPvxByLists(currIPv6, listsPair.v6);

            ++currPerfCount;
        }

        // На данном этапе
        // IPv4 / IPv6 - в status лежит состояние
        // Domain - в status лежит состояние, в removeIndicies индексы к удалению

        isFoundAny |= status;

        if (status && !applyFix) {
            // Record is valid, skipping
            continue;
        } else if (status) {
            if (isTypeDomain) {
                removeListItemsForInxs(domainBatch, removeIndicies);

                for (const auto& domain : domainBatch) {
                    fileTemp << buffer;
                }
            } else {
                fileTemp << buffer;
            }
        }

        if (isTypeDomain) {
            domainBatch.clear();
            currSize = 0;
        }

        if (status) {
            LOG_INFO("Detection in search between file and IP lists: " + buffer + " --> " + path.string());
        }

        progress = std::min(float(currPerfCount) / float(linesCount), 100.0f);
        logFilterCheckProgress(progress);
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
