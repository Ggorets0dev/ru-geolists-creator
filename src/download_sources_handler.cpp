#include "download_sources_handler.hpp"
#include "url_handle.hpp"
#include "log.hpp"
#include "ruadlist.hpp"
#include "filter.hpp"
#include "fs_utils_temp.hpp"

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
