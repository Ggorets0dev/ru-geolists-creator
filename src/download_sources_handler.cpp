#include "download_sources_handler.hpp"
#include "url_handle.hpp"
#include "log.hpp"
#include "ruadlist.hpp"
#include "filter.hpp"
#include "fs_utils_temp.hpp"

#define VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(condition) \
    if (!(condition)) { \
        LOG_ERROR(DOWNLOAD_UPDATES_FAIL_MSG); \
        return false; \
    }

static void filterDownloadedFiles(const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedFiles) {
    NetTypes::ListIPv4 ipv4;
    NetTypes::ListIPv6 ipv6;
    bool status;

    NetTypes::ListIPvxPair listsPair = {
        ipv4,
        ipv6
    };

    parseAddressFile(config.whitelistPath, listsPair);

    for (const auto& file : downloadedFiles) {
        LOG_INFO("Checking for whitelist entries: " + file.second.string());

        status = checkFileByIPvLists(file.second, listsPair, true);

        if (!status) {
            LOG_INFO("File [" + file.second.filename().string() + "] was checked successfully, no filter applied");
        } else {
            LOG_WARNING("File [" + file.second.filename().string() + "] was checked successfully, whitelist filter was applied");
        }
    }
}

bool downloadNewestSources(RgcConfig& config, const bool useExtraSources, const bool useFilter, std::vector<DownloadedSourcePair>& downloadedFiles) {
    bool status;
    Json::Value value;
    std::optional<std::time_t> lastReleaseTime;
    std::string latestRuadlistVersion;
    std::vector<std::string> assetsNames;
    std::vector<std::string> downloadsRefilter;
    std::vector<std::string> downloadsXray;
    size_t dupeCnt;

    // ========= Temp files control
    const FS::Utils::Temp::SessionTempFileRegistry tempFileReg;
    auto refilterReqFile = tempFileReg.createTempFileDetached("json");
    auto xrayReqFile = tempFileReg.createTempFileDetached("json");
    auto ruadlistReqFile = tempFileReg.createTempFileDetached("txt");
    auto ruadlistExtractedFile = tempFileReg.createTempFileDetached("txt");
    auto refilterFile = tempFileReg.createTempFileDetached("txt");
    // =========

    // SECTION - Download newest ReFilter rules
    status = NetUtils::tryDownloadFromGithub(REFILTER_API_LAST_RELEASE_URL, refilterReqFile->path, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download ReFilter lists API response");
        return false;
    }

    status = readJsonFromFile(refilterReqFile->path, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.refilterTime = *lastReleaseTime;

    assetsNames = {REFILTER_DOMAIN_ASSET_FILE_NAME, REFILTER_IP_ASSET_FILE_NAME};
    downloadsRefilter = NetUtils::downloadGithubReleaseAssets(value, assetsNames, tempFileReg.getTempDir());
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(downloadsRefilter.size() == assetsNames.size());

    downloadedFiles.emplace_back(Source(Source::Type::DOMAIN, REFILTER_SECTION_NAME), downloadsRefilter[0]);
    downloadedFiles.emplace_back(Source(Source::Type::IP, REFILTER_SECTION_NAME), downloadsRefilter[1]);

    LOG_INFO("ReFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest XRay rules
    status = NetUtils::tryDownloadFromGithub(XRAY_RULES_API_LAST_RELEASE_URL, xrayReqFile->path, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download XRay lists API response");
        return false;
    }

    status = readJsonFromFile(xrayReqFile->path, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.v2rayTime = *lastReleaseTime;

    assetsNames = {"reject-list.txt"};
    downloadsXray = NetUtils::downloadGithubReleaseAssets(value, assetsNames, tempFileReg.getTempDir());
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(downloadsXray.size() == assetsNames.size());

    downloadedFiles.emplace_back(Source(Source::Type::DOMAIN, XRAY_REJECT_SECTION_NAME), downloadsXray[0]);

    LOG_INFO("XRay rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest RUADLIST rules
    status = NetUtils::tryDownloadFromGithub(RUADLIST_API_MASTER_URL, ruadlistReqFile->path, config.apiToken);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList API response");
        return false;
    }

    status = readJsonFromFile(ruadlistReqFile->path, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = parseRuadlistUpdateDatetime(value, *lastReleaseTime);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = NetUtils::tryDownloadFile(RUADLIST_ADSERVERS_URL, ruadlistReqFile->path);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList AD servers");
        return false;
    }

    status = extractDomainsFromFile(ruadlistReqFile->path, ruadlistExtractedFile->path);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    removeDuplicateLines(ruadlistExtractedFile->path, downloadsXray[0]); // RuAdList vs reject-list.txt from XRay

    config.ruadlistTime = *lastReleaseTime;
    downloadedFiles.emplace_back(Source(Source::Type::DOMAIN, RUADLIST_SECTION_NAME), ruadlistExtractedFile->path);

    LOG_INFO("RuAdList rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest ANTIFILTER rules
    try {
        NetUtils::tryDownloadFile(ANTIFILTER_AYN_IPS_URL, refilterFile->path);

        dupeCnt = removeDuplicateLines(downloadsRefilter[1], refilterFile->path);

        if (dupeCnt) {
            LOG_INFO("Count of duplicates found between REFILTER and ANTIFILTER: " + std::to_string(dupeCnt));
        } else {
            LOG_INFO("No duplicates were found between REFILTER AND ANTIFILTER");
        }

        downloadedFiles.emplace_back(Source(Source::Type::IP, ANTIFILTER_SECTION_NAME), refilterFile->path);
    }  catch (std::exception& e) {
        LOG_ERROR(e.what());
        LOG_WARNING("Failed add ANTIFILTER rules to Geolists");
    }

    LOG_INFO("AntiFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download extra sources
    if (useExtraSources) {
        for (const auto& source : config.extraSources) {
            auto tempFile = tempFileReg.createTempFileDetached("txt");

            // ======== If path in system specified, no need to download
            if (!isUrl(source.url)) {
                try {
                    status = fs::exists(source.url);

                    if (status) {
                        fs::copy(source.url, tempFile->path);
                    }
                } catch (const fs::filesystem_error& e) {
                    LOG_ERROR("Filesystem error: " + std::string(e.what()));
                    status = false;
                }

                if (!status) {
                    LOG_WARNING("Failed to locate local extra source (ignored): " + source.url);
                    continue;
                }

                downloadedFiles.emplace_back(Source(source.type, source.section), tempFile->path);
                LOG_INFO("Local extra source was added successfully: " + source.url);
                continue;
            }
            // ========

            // ======== Downloading extra source via URL
            status = NetUtils::tryDownloadFile(source.url, tempFile->path);

            if (!status) {
                LOG_WARNING("Failed to downloaded extra source (ignored): " + source.url);
                continue;
            }

            downloadedFiles.emplace_back(Source(source.type, source.section), tempFile->path);
            LOG_INFO("Remote extra source was downloaded successfully: " + source.url);
            // ========
        }
    }
    // !SECTION

    // Filter files by whitelist if needed
    if (useFilter) {
        filterDownloadedFiles(config, downloadedFiles);
    }

    return true;
}
