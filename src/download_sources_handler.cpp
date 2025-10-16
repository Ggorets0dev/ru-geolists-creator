#include "download_sources_handler.hpp"
#include "network.hpp"
#include "log.hpp"
#include "ruadlist.hpp"
#include "filter.hpp"

#define VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(condition) \
    if (!condition) { \
        LOG_ERROR(DOWNLOAD_UPDATES_FAIL_MSG); \
        return false; \
    }

static void filterDownloadedFiles(RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedFiles) {
    NetTypes::ListIPv4 ipv4;
    NetTypes::ListIPv6 ipv6;
    bool status;

    parseAddressFile(config.whitelistPath, ipv4, ipv6);

    for (const auto& file : downloadedFiles) {
        LOG_INFO("Checking for whitelist entries: " + file.second.string());

        status = checkFileByIPvLists(file.second, ipv4, ipv6, true);

        if (!status) {
            LOG_INFO("File [" + file.second.filename().string() + "] was checked successfully, no filter applied");
        } else {
            LOG_WARNING("File [" + file.second.filename().string() + "] was checked successfully, whitelist filter was applied");
        }
    }
}

bool downloadNewestSources(RgcConfig& config, bool useExtraSources, bool useFilter, std::vector<DownloadedSourcePair>& downloadedFiles) {
    bool status;
    Json::Value value;
    std::optional<std::time_t> lastReleaseTime;
    std::string latestRuadlistVersion;
    std::vector<std::string> assetsNames;
    size_t dupeCnt;

    const fs::path kCurrentDir = fs::current_path();

    // SECTION - Download newest ReFilter rules
    status = tryDownloadFromGithub(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download ReFilter lists API response");
        return false;
    }

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.refilterTime = *lastReleaseTime;

    assetsNames = {REFILTER_DOMAIN_ASSET_FILE_NAME, REFILTER_IP_ASSET_FILE_NAME};
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[0]));
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::IP, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[1]));

    LOG_INFO("ReFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest XRay rules
    status = tryDownloadFromGithub(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download XRay lists API response");
        return false;
    }

    status = readJsonFromFile(XRAY_RULES_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.v2rayTime = *lastReleaseTime;

    assetsNames = {"reject-list.txt"};
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, XRAY_REJECT_SECTION_NAME), kCurrentDir / assetsNames[0]));

    LOG_INFO("XRay rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest RUADLIST rules
    status = tryDownloadFromGithub(RUADLIST_API_MASTER_URL, RUADLIST_FILE_NAME, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList API response");
        return false;
    }

    status = readJsonFromFile(RUADLIST_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = parseRuadlistUpdateDatetime(value, *lastReleaseTime);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = tryDownloadFile(RUADLIST_ADSERVERS_URL, RUADLIST_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList adservers");
        return false;
    }

    status = extractDomainsFromFile(RUADLIST_FILE_NAME, RUADLIST_EXTRACTED_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    removeDuplicateLines(RUADLIST_EXTRACTED_FILE_NAME, assetsNames[0]); // RuAdList vs reject-list.txt from XRay

    config.ruadlistTime = *lastReleaseTime;
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, RUADLIST_SECTION_NAME), kCurrentDir / RUADLIST_EXTRACTED_FILE_NAME));

    LOG_INFO("RuAdList rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest ANTIFILTER rules
    try {
        tryDownloadFile(ANTIFILTER_AYN_IPS_URL, ANTIFILTER_FILE_NAME);

        dupeCnt = removeDuplicateLines(REFILTER_IP_ASSET_FILE_NAME, ANTIFILTER_FILE_NAME);

        if (dupeCnt) {
            LOG_INFO("Count of duplicates found between REFILTER and ANTIFILTER: " + std::to_string(dupeCnt));
        } else {
            LOG_INFO("No duplicates were found between REFILTER AND ANTIFILTER");
        }

        downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::IP, ANTIFILTER_SECTION_NAME), kCurrentDir / ANTIFILTER_FILE_NAME));
    }  catch (std::exception& e) {
        LOG_ERROR(e.what());
        LOG_WARNING("Failed add Antifilter rules to Geolists");
    }

    LOG_INFO("AntiFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download extra sources
    std::string fileName;

    for (const auto& source : config.extraSources) {
        fileName = genSourceFileName(source);
        status = tryDownloadFile(source.url, fileName);

        if (!status) {
            LOG_WARNING("Failed to downloaded extra source: " + source.url);
            continue;
        }

        downloadedFiles.push_back(DownloadedSourcePair(Source(source.type, source.section), kCurrentDir / fileName));
        LOG_INFO("Extra source was downloaded successfully: " + source.url);
    }
    // !SECTION

    // Filter files by whitelist if needed
    if (useFilter) {
        filterDownloadedFiles(config, downloadedFiles);
    }

    return true;
}
