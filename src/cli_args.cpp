#include "cli_args.hpp" 

#include "time_tools.hpp"
#include "software_info.hpp"
#include "log.hpp"

#define FORCE_OPTION_DESCRIPTION                "Starts source download and build even if no updates are detected"
#define ABOUT_OPTION_DESCRIPTION                "Display software information"
#define CHECK_OPTION_DESCRIPTION                "Check access of all source's URLs from config"
#define INIT_OPTION_DESCRIPTION                 "Initialize software by creating config and downloading all dependencies"
#define ADD_EXTRA_OPTION_DESCRIPTION            "Add extra source to download list"
#define REMOVE_EXTRA_OPTION_DESCRIPTION         "Remove extra source from download list"
#define OUT_DIR_OPTION_DESCRIPTION              "Path to out DIR with all lists to create"
#define FORMATS_OPTION_DESCRIPTION              "Formats of geolists to generate (v2ray, sing)"
#define WHITELIST_OPTION_DESCRIPTION            "Enable whitelist filtering for current session"
#define NO_EXTRA_OPTION_DESCRIPTION             "Disable adding extra sources to lists for current session"
#define OUT_PATH_OPT_GRP_DESCRIPTION            "Set path for build results"

#define ASK_MARK                                "❔"

#define SHOW_SUBCMD_DESC                        "Display all extra sources from config files"
#define SORT_EXTRAS_BY_SECTION_DESCRIPTION      "Sort extra sources by section name"
#define SORT_EXTRAS_BY_TYPE_DESCRIPTION         "Sort extra sources by type"

#define SERVICE_SUBCMD_DESC                     "Settings for service mode"
#define SERVICE_ADDR_OPT_DESC                   "IP address for service"
#define SERVICE_PORT_OPT_DESC                   "System port for service"
#define SERVICE_TIMEOUT_OPT_DESC                "Timeout (sec) for service watchdog (leads to shutdown, 0 is infinite work)"

CmdArgs gCmdArgs = {};
ServiceSettings gServiceSettings = {};

CLI::App* gServiceSubCmd;
CLI::App* gShowSubCmd;
CLI::Option* gRemoveExtraOption;
CLI::Option* gOutPathOption;

static const CLI::Range gkServicePortRange = { 49152, 65535 };
static const CLI::Range gkServiceTimeoutRange = { 0, HOURS_TO_SEC(1) };

static const std::vector<std::string> gkAvailableGeoFormats = {
    GEO_FORMAT_V2RAY_CAPTION,
    GEO_FORMAT_SING_CAPTION
};

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

    const auto sortExtrasByType = gShowSubCmd->add_flag(
        "--sort-type",
        gCmdArgs.isSortExtrasByTypes,
        SORT_EXTRAS_BY_TYPE_DESCRIPTION);

    const auto sortExtrasBySection = gShowSubCmd->add_flag(
        "--sort-sec",
        gCmdArgs.isSortExtrasBySections,
        SORT_EXTRAS_BY_SECTION_DESCRIPTION);

    // Only one type of extra's sort can be selected
    sortExtrasByType->excludes(sortExtrasBySection);
}

void prepareCmdArgs(CLI::App& app) {
    app.description(RGC_DESCRIPTION);

    setupServiceSubcommand(app);
    setupShowSubcommand(app);

    app.add_flag("--force", gCmdArgs.isForceCreation, FORCE_OPTION_DESCRIPTION);
    app.add_flag("--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
    app.add_flag("--check", gCmdArgs.isCheckUrls, CHECK_OPTION_DESCRIPTION);
    app.add_flag("--init", gCmdArgs.isInit, INIT_OPTION_DESCRIPTION);

    app.add_flag("-a, --add", gCmdArgs.isAddExtra, ADD_EXTRA_OPTION_DESCRIPTION);

    app.add_flag("--whitelist", gCmdArgs.isUseWhitelist, WHITELIST_OPTION_DESCRIPTION);
    app.add_flag("--no-extra", gCmdArgs.isNoExtra, NO_EXTRA_OPTION_DESCRIPTION);

    app.add_option("-f,--format", gCmdArgs.formats, FORMATS_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);

    gRemoveExtraOption = app.add_option("-r, --remove", gCmdArgs.extraSourceId, REMOVE_EXTRA_OPTION_DESCRIPTION);

    gOutPathOption = app.add_option("-o, --out", gCmdArgs.outDirPath, OUT_DIR_OPTION_DESCRIPTION)
        ->capture_default_str();
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

bool askYesNo(const std::string& question, bool isYesDefault) {
    std::string userChoice;
    userChoice.reserve(1);

    const char yesChar = isYesDefault ? 'Y' : 'y';
    const char noChar = isYesDefault ? 'n' : 'N';

    while (true) {
        std::cout << ASK_MARK << " " << question << " (" << yesChar << "/" << noChar << "): ";
        std::getline(std::cin, userChoice);

        if (userChoice.empty()) {
            return isYesDefault;
        } else if (userChoice.length() == 1 && std::tolower(userChoice[0]) == 'y' ||
                   std::tolower(userChoice[0]) == 'n') {

            return std::tolower(userChoice[0]) == 'y';
        }
    }
}

void getStringInput(const std::string& question, std::string& out, bool isEmptyAllowed) {
    while (true) {
        std::cout << ASK_MARK << " " << question << ": ";
        std::getline(std::cin, out);

        if (isEmptyAllowed || !out.empty()) {
            break;
        }
    }
}
