#include <set>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>

#include "filter.hpp"
#include "log.hpp"
#include "common.hpp"
#include "net_convert.hpp"
#include "url_handle.hpp"
#include "cares_resolver.hpp"
#include "config.hpp"

#define FILTER_FILENAME_POSTFIX     "temp_filter"

static bool parseAddress(const std::string& buffer, const NetTypes::ListIPvxPair& listsPair, NetTypes::ListAddress* domainBuffer=nullptr) {
    bool status;
    NetTypes::AddressType type;
    NetTypes::IPv4Subnet bufferIPv4;
    NetTypes::IPv6Subnet bufferIPv6;

    type = NetUtils::getAddressType(buffer);

    if (type == NetTypes::AddressType::IPV4) {
        status = NetUtils::Convert::parseIPv4(buffer, bufferIPv4);
        if (status) {
            listsPair.v4.push_front(bufferIPv4);
        }
    } else if (type == NetTypes::AddressType::IPV6) {
        status = NetUtils::Convert::parseIPv6(buffer, bufferIPv6);
        if (status) {
            listsPair.v6.push_front(bufferIPv6);
        }
    } else if (type == NetTypes::AddressType::DOMAIN && (domainBuffer != nullptr)) {
        domainBuffer->push_front(buffer);
    } else if (type == NetTypes::AddressType::DOMAIN) {
        // Nothing to do, skipping
    } else { // NetTypes::AddressType::UNKNOWN
        // Failed to get address type
        return false;
    }

    return true;
}

static bool parseAddress(const NetTypes::ListAddress& addrs, const NetTypes::ListIPvxPair& listsPair) {
    bool status = true;

    for (const auto& addr : addrs) {
        status &= parseAddress(addr, listsPair);
    }

    return status;
}

void parseAddressFile(const fs::path& path, NetTypes::ListIPvxPair& listsPair) {
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
        status = parseAddress(buffer, listsPair, &domainsBuffer);

        if (!status) {
            LOG_WARNING("An unknown entry was found in file with addresses, type could not be determined: " + buffer);
        }
    }

    // ======== Convert all domains to IPv4 or IPv6
    NetUtils::CAresResolver resolver;

    if (!resolver.isInitialized()) {
        std::cerr << "Failed to init resolver\n";
        return;
    }

    resolver.resolveDomains(domainsBuffer, uniqueIPs);

    domainsBuffer.clear();

    for (const auto& ip : uniqueIPs) {
        parseAddress(ip, listsPair);
    }
    // ========

    ipv4Size = std::distance(listsPair.v4.begin(), listsPair.v4.end());
    ipv6Size = std::distance(listsPair.v6.begin(), listsPair.v6.end());

    LOG_INFO("File " + path.string() + " parsed to " + std::to_string(ipv4Size) + " IPv4 entities and " + std::to_string(ipv6Size) + " IPv6 entities");
}

bool isUrl(const std::string& str) {
    static const std::regex url_regex(
        R"(^[a-zA-Z][a-zA-Z0-9+.\-]*://)"
    );
    return std::regex_search(str, url_regex);
}

template <typename T>
static bool checkIPvxByLists(NetTypes::ListIPvx<T>& current, const NetTypes::ListIPvx<T>& lists, std::forward_list<uint32_t>* removeInxs=nullptr) {
    bool checkResult = false;
    size_t index = 0;

    for (const auto& cIP : current) {
        for (const auto& lIP : lists) {
            if (const bool status = lIP.isSubnetIncludes(cIP); status && (removeInxs == nullptr)) {
                return true;
            } else if (status) {
                checkResult = true;
                removeInxs->push_front(index);

                break;
            }
        }
        ++index;
    }

    return checkResult;
}


bool checkFileByIPvLists(const fs::path& path, const NetTypes::ListIPvxPair& listsPair, bool applyFix) {
    const fs::path tempFilePath = addPathPostfix(path, FILTER_FILENAME_POSTFIX);

    // listsPair is from whitelist (in most cases)

    //  ======= Variables for file, which will be checked
    NetTypes::ListIPv4 currIPv4;
    NetTypes::ListIPv6 currIPv6;

    NetTypes::ListIPvxPair currListsPair = {
        currIPv4,
        currIPv6
    };

    NetTypes::ListAddress uniqueIPs;
    // =======

    std::ifstream file;
    std::ofstream fileTemp;

    std::string buffer;
    NetTypes::ListAddress domainBatch;

    std::forward_list<uint32_t> removeInxs;

    bool status;
    bool isFoundAny(false);

    float progress;
    const size_t linesCount = countLinesInFile(path);
    size_t currSize(0);
    size_t currPerfCount(0);

    NetUtils::CAresResolver resolver;

    if (!resolver.isInitialized()) {
        LOG_ERROR("Failed to init CAres domain resolver");
        return false;
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
        auto type = NetUtils::getAddressType(buffer);
        bool isTypeDomain = type == NetTypes::AddressType::DOMAIN;

        status = false;

        removeInxs.clear();

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
            parseAddress(uniqueIPs, currListsPair);

            status |= checkIPvxByLists(currIPv4, listsPair.v4, &removeInxs);
            status |= checkIPvxByLists(currIPv6, listsPair.v6, &removeInxs);

            currPerfCount += currSize;
        } else if (isTypeDomain) {
            // Not enough recording in batch, skipping
            continue;
        } else {
            parseAddress(buffer, currListsPair);

            status |= checkIPvxByLists(currIPv4, listsPair.v4);
            status |= checkIPvxByLists(currIPv6, listsPair.v6);

            ++currPerfCount;
        }

        // На данном этапе
        // IPv4 / IPv6 - в status лежит состояние
        // Domain - в status лежит состояние, в removeInxs индексы к удалению

        isFoundAny |= status;

        if (!status && !applyFix) {
            // Record is valid, skipping
            continue;
        } else if (!status) {
            if (isTypeDomain) {
                removeListItemsForInxs(domainBatch, removeInxs);

                for (const auto& domain : domainBatch) {
                    fileTemp << domain << '\n';
                }
            } else {
                fileTemp << buffer << '\n';
            }
        }

        if (isTypeDomain) {
            domainBatch.clear();
            currSize = 0;
        }

        if (status && isTypeDomain) {
            LOG_INFO("Detection in search between file and IP lists: domain batch");
        } else if (status) {
            LOG_INFO("Detection in search between file and IP lists: " + buffer + " --> " + path.string());
        }

        progress = std::min(static_cast<float>(currPerfCount) / static_cast<float>(linesCount), 100.0f);
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

void filterDownloadsByWhitelist(const std::vector<DownloadedSourcePair>& downloadedFiles) {
    NetTypes::ListIPv4 ipv4;
    NetTypes::ListIPv6 ipv6;

    NetTypes::ListIPvxPair listsPair = {
        ipv4,
        ipv6
    };

    const auto config = getCachedConfig();

    parseAddressFile(config->whitelistPath, listsPair);

    for (const auto&[fst, snd] : downloadedFiles) {
        LOG_INFO("Checking for whitelist entries: " + snd.string());

        if (const bool status = checkFileByIPvLists(snd, listsPair, true); !status) {
            LOG_INFO("File [" + snd.filename().string() + "] was checked successfully, no filter applied");
        } else {
            LOG_WARNING("File [" + snd.filename().string() + "] was checked successfully, whitelist filter was applied");
        }
    }
}

bool extractDomainsInPlace(const std::string& filePath) {
    if (!fs::exists(filePath)) {
        LOG_ERROR("File does not exist: {}", filePath);
        return false;
    }

    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        LOG_ERROR("Failed to open file for reading: {}", filePath);
        return false;
    }

    std::set<std::string> domains;
    static const std::regex kDomainSearcher(R"(([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,63})");

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty() || line[0] == '!' || line[0] == '#') {
            continue;
        }

        std::smatch match;
        if (std::regex_search(line, match, kDomainSearcher)) {
            std::string found = match.str();
            std::transform(found.begin(), found.end(), found.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            domains.insert(found);
        }
    }
    inputFile.close();

    std::ofstream outputFile(filePath, std::ios::trunc);
    if (!outputFile.is_open()) {
        LOG_ERROR("Failed to open file for writing: {}", filePath);
        return false;
    }

    for (const auto& domain : domains) {
        outputFile << domain << "\n";
    }

    LOG_INFO("Processing domain extraction complete. Path: {}, Unique domains: {}", filePath, domains.size());

    return true;
}