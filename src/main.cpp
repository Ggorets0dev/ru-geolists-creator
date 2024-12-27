#include "handlers.hpp"
#include "config.hpp"

int main() {
    if (!fs::exists(RGC_CONFIG_PATH)) {
        LOG_WARNING("Configuration file is not detected, initialization is performed");
        initSoftware(); // Download all toolchains and create config
    }

    auto updatesStatus = checkForUpdates();
}
