#include "handlers.hpp"
#include "config.hpp"
#include "temp.hpp"
#include "main_sources.hpp"

static void
printDownloadedSources(const std::vector<DownloadedSourcePair>& downloadedSources) {
    std::cout << "---------" << std::endl;

    for (const auto& source : downloadedSources) {
        source.first.print();
        std::cout << "Path: " << source.second << std::endl;
        std::cout << "---------" << std::endl;
    }
}

int main() {
    bool status;
    RgcConfig config;
    std::vector<DownloadedSourcePair> downloadedSources;

    if (!fs::exists(RGC_CONFIG_PATH)) {
        LOG_WARNING("Configuration file is not detected, initialization is performed");
        initSoftware(); // Download all toolchains and create config

        LOG_INFO("You can add a GitHub API access key before running the software. Restart the application with the token added if desired");
        return 0;
    }

    status = readConfig(config);
    if (!status) {
        LOG_ERROR("Configuration file could not be read, operation cannot be continued");
        return 1;
    }

    CREATE_TEMP_DIR();
    ENTER_TEMP_DIR();

    auto [checkStatus, isUpdateFound] = checkForUpdates(config);

    if (!checkStatus) {
        // An additional log can be posted here
        return 1; // Failed to check updates. Exit
    }

    if (!isUpdateFound) {
        LOG_INFO("No need to update sources, exit the program");
        return 0;
    }

    LOG_INFO("Process of downloading the latest versions of the sources begins...");
    status = downloadNewestSources(config, true, downloadedSources);

    if (!status) {
        // An additional log can be posted here
        return 1; // Failed to download newest releases. Exit
    }

    EXIT_TEMP_DIR();

    LOG_INFO("Successfully downloaded all sources: \n");
    printDownloadedSources(downloadedSources);

    writeConfig(config);

    // SECTION - Move sources to toolchains
    std::vector<std::string> v2ipSections;
    Json::Value v2ipInputRules;

    for (const auto& source : downloadedSources) {
        if (source.first.type == Source::Type::DOMAIN) {
            status &= addDomainSource(config.dlcRootPath, source.second);
        } else { // IP
            addIPSource(source, v2ipInputRules);
        }
    }

    status &= saveIPSources(config.v2ipRootPath, v2ipInputRules, v2ipSections);

    if (!status) {
        LOG_ERROR("Failed to correctly place sources in toolchains");
        return 1;
    }
    // !SECTION

    return 0;
}
