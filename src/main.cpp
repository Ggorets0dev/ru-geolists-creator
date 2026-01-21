#include "handlers.hpp"
#include "config.hpp"
#include "cli_args.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "service_callbacks.hpp"

int main(const int argc, char** argv) {
    CLI::App app;
    RgcConfig config;
    int exitCode = 0;

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
            if (askYesNo("Initialization is already performed, delete config and run it again?", false)) {
                deinitSoftware();
            } else {
                LOG_INFO("Re-initialization canceled");
                return exitCode;
            }
        }

        LOG_INFO("Launching software initialization...");

        initSoftware(); // Download all toolchains and create config

        return exitCode;
    }

    // Print software information
    if (gCmdArgs.isShowAbout) {
        printAbout();
        return exitCode;
    }

    if (!fs::exists(gkConfigPath)) {
        LOG_WARNING("Configuration file is not found, perform software initialization using --init");
        return 1;
    }

    if (!readConfig(config) || !validateConfig(config)) {
        LOG_ERROR(READ_CFG_FAIL_MSG);
        return 1;
    }

    // Global cache, which will be used in functions
    // TODO: Add config validation
    setCachedConfig(config);

    if (app.got_subcommand(gServiceSubCmd)) {
        fillServiceCallbacks(gServiceCallbacks);
        runService(gServiceSettings, gServiceCallbacks);
    }

    // Show all extra sources
    if (app.got_subcommand(gShowSubCmd)) {
        showPresets(gCmdArgs);
    }

    // Build sources from presets
    if (app.got_subcommand(gBuildSubCmd)) {
        const auto releases = buildListsHandler(gCmdArgs);
        const bool status = releases != std::nullopt;
        exitCode = !status;
    }

    // Check access for URLs and close software
    if (app.got_subcommand(gCheckSubCmd)) {
        checkUrlsAccess(gCmdArgs);
    }

    return exitCode;
}
