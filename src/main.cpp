#include "handlers.hpp"
#include "config.hpp"

#define TEMP_DIR_PATH   "./temp"

int main() {
    bool status;
    RgcConfig config;
    std::vector<fs::path> downloadedSources;

    if (!fs::exists(RGC_CONFIG_PATH)) {
        LOG_WARNING("Configuration file is not detected, initialization is performed");
        initSoftware(); // Download all toolchains and create config
    }

    status = readConfig(config);
    if (!status) {
        LOG_ERROR("Configuration file could not be read, operation cannot be continued");
        return 1;
    }

    if (!fs::exists(TEMP_DIR_PATH)) {
        mkdir(TEMP_DIR_PATH, 0755);
    }
    chdir(TEMP_DIR_PATH);

    auto [checkStatus, isUpdateFound] = checkForUpdates(config);

    if (!checkStatus) {
        // An additional log can be posted here
        return 1; // Failed to check updates. Exit
    }

    if (!isUpdateFound) {
        LOG_INFO("No need to update sources, exit the program");
        return 0;
    }

    LOG_INFO("Process of downloading the latest versions of the sources begins");
    status = downloadNewestSources(config, true, downloadedSources);

    if (!status) {
        // An additional log can be posted here
        return 1; // Failed to download newest releases. Exit
    }

    LOG_INFO("Successfully downloaded all sources");

    chdir("..");
    writeConfig(config);

    return 0;
}
