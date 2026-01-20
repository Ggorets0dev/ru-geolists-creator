#include "cli_args.hpp" 

#include "time_tools.hpp"
#include "software_info.hpp"
#include "log.hpp"

#define ABOUT_OPTION_DESCRIPTION                "Display software information"
#define INIT_OPTION_DESCRIPTION                 "Initialize software by creating config and downloading all dependencies"

#define ASK_MARK                                "❔"

#define BUILD_SUBCMD_DESC                       "Build geofiles with selected presets"
#define OUT_DIR_OPTION_DESCRIPTION              "Path to out DIR with all lists to create"
#define PRESET_OPTION_DESCRIPTION               "Presets from config for check/build"
#define FORMATS_OPTION_DESCRIPTION              "Formats of geolists to generate (v2ray, sing)"
#define WHITELIST_OPTION_DESCRIPTION            "Enable whitelist filtering for current session"

#define CHECK_SUBCMD_DESC                       "Check access of all source's URLs from config"

#define SHOW_SUBCMD_DESC                        "Display all extra sources from config files"
#define SORT_SOURCES_BY_SECTION_DESCRIPTION         "Sort sources by section name"
#define SORT_SOURCES_BY_STORAGE_TYPE_DESCRIPTION    "Sort sources by storage type"
#define SORT_SOURCES_BY_INET_TYPE_DESCRIPTION       "Sort sources by inet type"

#define SERVICE_SUBCMD_DESC                     "Settings for service mode"
#define SERVICE_ADDR_OPT_DESC                   "IP address for service"
#define SERVICE_PORT_OPT_DESC                   "System port for service"
#define SERVICE_TIMEOUT_OPT_DESC                "Timeout (sec) for service watchdog (leads to shutdown, 0 is infinite work)"

CmdArgs gCmdArgs = {};
ServiceSettings gServiceSettings = {};

CLI::App* gServiceSubCmd;
CLI::App* gBuildSubCmd;
CLI::App* gCheckSubCmd;
CLI::App* gShowSubCmd;
CLI::Option* gOutPathOption;

static const CLI::Range gkServicePortRange = { 49152, 65535 };
static const CLI::Range gkServiceTimeoutRange = { 0, HOURS_TO_SEC(1) };

static const std::vector<std::string> gkAvailableGeoFormats = {
    GEO_FORMAT_V2RAY_CAPTION,
    GEO_FORMAT_SING_CAPTION
};

static void setupCheckSubcommand(CLI::App& app) {
    gCheckSubCmd = app.add_subcommand("check", CHECK_SUBCMD_DESC);

    gCheckSubCmd->add_option("-p, --preset", gCmdArgs.presets, PRESET_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
}

static void setupBuildSubcommand(CLI::App& app) {
    gBuildSubCmd = app.add_subcommand("build", BUILD_SUBCMD_DESC);

    gBuildSubCmd->add_flag("--whitelist", gCmdArgs.isUseWhitelist, WHITELIST_OPTION_DESCRIPTION);

    gBuildSubCmd->add_option("-f,--format", gCmdArgs.formats, FORMATS_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);

    gBuildSubCmd->add_option("-o, --out", gCmdArgs.outDirPath, OUT_DIR_OPTION_DESCRIPTION)
        ->capture_default_str();

    gBuildSubCmd->add_option("-p, --preset", gCmdArgs.presets, PRESET_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
}

static void setupServiceSubcommand(CLI::App& app) {
    gServiceSubCmd = app.add_subcommand("service", SERVICE_SUBCMD_DESC);

    gServiceSubCmd->add_option("-a,--addr", gServiceSettings.addr, SERVICE_ADDR_OPT_DESC)
               ->capture_default_str();

    gServiceSubCmd->add_option("-p,--port", gServiceSettings.port, SERVICE_PORT_OPT_DESC)
               ->check(gkServicePortRange) // Extra port validation
               ->capture_default_str();

    gServiceSubCmd->add_option("-t,--timeout", gServiceSettings.timeout_sec, SERVICE_TIMEOUT_OPT_DESC)
                   ->check(gkServiceTimeoutRange) // Extra timeout validation
                   ->capture_default_str();
}

static void setupShowSubcommand(CLI::App& app) {
    gShowSubCmd = app.add_subcommand("show", SHOW_SUBCMD_DESC);

    const auto sortByInetTypeFlag = gShowSubCmd->add_flag(
        "--sort-inet",
        gCmdArgs.isSortByeInetTypes,
        SORT_SOURCES_BY_INET_TYPE_DESCRIPTION);

    const auto sortByStorageType = gShowSubCmd->add_flag(
        "--sort-storage",
        gCmdArgs.isSortByStorageTypes,
        SORT_SOURCES_BY_STORAGE_TYPE_DESCRIPTION);

    const auto sortBySecFlag = gShowSubCmd->add_flag(
        "--sort-section",
        gCmdArgs.isSortBySections,
        SORT_SOURCES_BY_SECTION_DESCRIPTION);

    // Only one type of extra's sort can be selected
    sortByInetTypeFlag->excludes(sortByStorageType);
    sortByInetTypeFlag->excludes(sortBySecFlag);
    sortBySecFlag->excludes(sortByStorageType);

    gShowSubCmd->add_option("-p, --preset", gCmdArgs.presets, PRESET_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
}

void prepareCmdArgs(CLI::App& app) {
    app.description(RGC_DESCRIPTION);

    setupServiceSubcommand(app);
    setupShowSubcommand(app);
    setupBuildSubcommand(app);
    setupCheckSubcommand(app);

    app.add_flag("--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
    app.add_flag("--init", gCmdArgs.isInit, INIT_OPTION_DESCRIPTION);
}

bool validateParsedFormats(const CmdArgs& args) {
    if (args.formats.empty()) {
        LOG_ERROR("No format specified for geolists");
        return false;
    }

    for (const auto& format : args.formats) {
        if (std::find(gkAvailableGeoFormats.begin(), gkAvailableGeoFormats.end(), format) == gkAvailableGeoFormats.end()) {
            // Unsopported format for geolists
            LOG_ERROR("Unsupported format specified: " + format);
            return false;
        }
    }

    return true;
}

bool askYesNo(const std::string& question, const bool isYesDefault) {
    std::string userChoice;
    userChoice.reserve(1);

    const char yesChar = isYesDefault ? 'Y' : 'y';
    const char noChar = isYesDefault ? 'n' : 'N';

    while (true) {
        std::cout << ASK_MARK << " " << question << " (" << yesChar << "/" << noChar << "): ";
        std::getline(std::cin, userChoice);

        if (userChoice.empty()) {
            return isYesDefault;
        }

        if (userChoice.length() == 1 && std::tolower(userChoice[0]) == 'y' || std::tolower(userChoice[0]) == 'n') {
            return std::tolower(userChoice[0]) == 'y';
        }
    }
}

void getStringInput(const std::string& question, std::string& out, const bool isEmptyAllowed) {
    while (true) {
        std::cout << ASK_MARK << " " << question << ": ";
        std::getline(std::cin, out);

        if (isEmptyAllowed || !out.empty()) {
            break;
        }
    }
}
