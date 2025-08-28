#include "cli_args.hpp" 

#include "log.hpp"
#include "config.hpp"
#include "network.hpp"
#include "main_sources.hpp"
#include "extra_sources.hpp"

#include "software_info.hpp"

#define FORCE_OPTION_DESCRIPTION    "Starts source download and build even if no updates are detected"
#define ABOUT_OPTION_DESCRIPTION    "Displaying software information"
#define CHECK_OPTION_DESCRIPTION    "Checking access of all source's URLs from config"
#define CHILD_OPTION_DESCRIPTION    "Sending release notes to parent proccess (for work in chain)"

CmdArgs gCmdArgs = { 0 };

void
printSoftwareInfo() {
    std::cout << "ru-geolists-creator v" << RGC_VERSION << std::endl;
    std::cout << "Developer: " << RGC_DEVELOPER << std::endl;
    std::cout << "License: " << RGC_LICENSE << std::endl;
    std::cout << "GitHub: " << RGC_REPOSITORY << std::endl;
}

void
checkUrlsAccess() {
    bool access_status;
    RgcConfig config;

    // Adding all main sources
    std::vector<std::string> urls = {
        REFILTER_API_LAST_RELEASE_URL,
        XRAY_RULES_API_LAST_RELEASE_URL,
        RUADLIST_API_MASTER_URL,
        RUADLIST_ADSERVERS_URL,
        ANTIFILTER_AYN_IPS_URL
    };

	LOG_INFO("Checking all sources for access via web...\n");

    const bool cfg_read_status = readConfig(config);

    if (cfg_read_status) {
        std::transform(config.extraSources.begin(), config.extraSources.end(), std::back_inserter(urls),
                       [](const ExtraSource& source) { return source.url; });
    } else {
        LOG_WARNING("Configuration file could not be read, extra sources wont be checked");
    }

    for (const std::string& url : urls) {
        access_status = isUrlAccessible(url);
        log_url_access(url, access_status);
    }
}

void
prepareCmdArgs(CLI::App& app, int argc, char** argv) {
    app.description(RGC_DESCRIPTION);

    argv = app.ensure_utf8(argv);

    app.add_flag("-f,--force", gCmdArgs.isForceCreation, FORCE_OPTION_DESCRIPTION);
    app.add_flag("-a,--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
    app.add_flag("-c,--check", gCmdArgs.isCheckUrls, CHECK_OPTION_DESCRIPTION);
    app.add_flag("--child", gCmdArgs.isChild, CHILD_OPTION_DESCRIPTION);
}
