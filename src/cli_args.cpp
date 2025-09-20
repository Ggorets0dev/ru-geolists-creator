#include "cli_args.hpp" 

#include "software_info.hpp"
#include "log.hpp"

#include <iterator>

#define FORCE_OPTION_DESCRIPTION                "Starts source download and build even if no updates are detected"
#define ABOUT_OPTION_DESCRIPTION                "Displaying software information"
#define CHECK_OPTION_DESCRIPTION                "Checking access of all source's URLs from config"
#define CHILD_OPTION_DESCRIPTION                "Sending release notes to parent proccess (for work in chain)"
#define INIT_OPTION_DESCRIPTION                 "Initializing software by creating config and downloading all dependencies"
#define SHOW_OPTION_DESCRIPTION                 "Displaying all extra sources from configuration files"
#define ADD_EXTRA_OPTION_DESCRIPTION            "Adding extra source to download list"
#define REMOVE_EXTRA_OPTION_DESCRIPTION         "Removing extra source from download list"
#define OUT_DIR_OPTION_DESCRIPTION              "Path to out DIR with all lists to create"
#define FORMATS_OPTION_DESCRIPTION              "Formats of geolists to generate"

#define OUT_PATH_OPT_GRP_DESCRIPTION            "Set path for build results"

#define ASK_MARK "❔"

CmdArgs gCmdArgs = { 0 };

CLI::Option* gRemoveExtraOption;
CLI::Option* gOutPathOption;

static const std::vector<std::string> gkAvailableGeoFormats = {
    GEO_FORMAT_V2RAY_CAPTION,
    GEO_FORMAT_SING_CAPTION
};

void prepareCmdArgs(CLI::App& app, int argc, char** argv) {
    app.description(RGC_DESCRIPTION);

    argv = app.ensure_utf8(argv);

    (void)argv;

    app.add_flag("--force", gCmdArgs.isForceCreation, FORCE_OPTION_DESCRIPTION);
    app.add_flag("--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
    app.add_flag("--check", gCmdArgs.isCheckUrls, CHECK_OPTION_DESCRIPTION);
    app.add_flag("--child", gCmdArgs.isChild, CHILD_OPTION_DESCRIPTION);
    app.add_flag("--init", gCmdArgs.isInit, INIT_OPTION_DESCRIPTION);

    app.add_flag("--show", gCmdArgs.isShowExtras, SHOW_OPTION_DESCRIPTION);
    app.add_flag("-a, --add", gCmdArgs.isAddExtra, ADD_EXTRA_OPTION_DESCRIPTION);

    app.add_option("-f,--format", gCmdArgs.formats, FORMATS_OPTION_DESCRIPTION)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);

    gRemoveExtraOption = app.add_option("-r, --remove", gCmdArgs.extraSourceId, REMOVE_EXTRA_OPTION_DESCRIPTION);
    gOutPathOption = app.add_option("-o, --out", gCmdArgs.outDirPath, OUT_DIR_OPTION_DESCRIPTION);
}

void printAvailableFormats() {
    std::cout << "Available formats of geolists: ";

    for (uint8_t i(0); i < gkAvailableGeoFormats.size(); ++i) {
        std::cout << gkAvailableGeoFormats[i];

        if (i + 1 < gkAvailableGeoFormats.size()) {
            std::cout << ", ";
        }
    }

    std::cout << std::endl;
}

bool validateParsedFormats() {
    if (gCmdArgs.formats.empty()) {
        LOG_ERROR("No format specified for geolists");
        return false;
    }

    for (const auto& format : gCmdArgs.formats) {
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

    char yesChar = isYesDefault ? 'Y' : 'y';
    char noChar = isYesDefault ? 'n' : 'N';

    while (true) {
        std::cout << ASK_MARK << " " << question << " (" << yesChar << "/" << noChar << "): ";
        std::getline(std::cin, userChoice);

        if (userChoice.length() == 0) {
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

        if (isEmptyAllowed || out.length() > 0) {
            break;
        }
    }
}
