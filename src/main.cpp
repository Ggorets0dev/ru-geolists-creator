#include "handlers.hpp"
#include "config.hpp"
#include "cli_args.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "service_callbacks.hpp"

int main(const int argc, char** argv) {
    CLI::App app;
    RgcConfig config;

    // Init RAND
    const auto now = std::chrono::high_resolution_clock::now();
    const auto nanos = now.time_since_epoch().count();
    std::srand(static_cast<unsigned int>(nanos ^ (nanos >> 32)));

    // Init logging
    initLogging();

    // SECTION - Parse CMD args using CLI11 lib
    prepareCmdArgs(app);

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        printHelp(app);
        return 0;
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }
    // !SECTION

    // Show about and close software
    if (gCmdArgs.isInit) {
        if (fs::exists(gkConfigPath)) {
            if (bool isInitAgain = askYesNo("Initialization is already performed, delete config and run it again?", false)) {
                deinitSoftware();
            } else {
                LOG_INFO("Re-initialization canceled");
                return 0;
            }
        }

        LOG_INFO("Launching software initialization...");

        initSoftware(); // Download all toolchains and create config

        return 0;
    }

    // Print software information
    if (gCmdArgs.isShowAbout) {
        printAbout();
        return 0;
    }

    if (!fs::exists(gkConfigPath)) {
        LOG_WARNING("Configuration file is not found, perform software initialization using --init");
        return 0;
    }

    if (app.got_subcommand(gServiceSubCmd)) {
        fillServiceCallbacks(gServiceCallbacks);
        runService(gServiceSettings, gServiceCallbacks);
        return 0;
    }

    // Add extra source
    if (gCmdArgs.isAddExtra) {
        addExtraSource();
        return 0;
    }

    // Remove extra source
    if (gRemoveExtraOption->count() == 1) {
        removeExtraSource(gCmdArgs.extraSourceId);
        return 0;
    }

    // Show all extra sources
    if (app.got_subcommand(gShowSubCmd)) {
        showExtraSources();
        return 0;
    }

    // Check access for URLs and close software
    if (gCmdArgs.isCheckUrls) {
        checkUrlsAccess();
        return 0;
    }

    // NOTE: Main job
    const auto releases = buildListsHandler(gCmdArgs);
    const bool status = releases != std::nullopt;
    return !status;
}
