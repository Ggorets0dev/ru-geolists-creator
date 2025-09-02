#include "cli_args.hpp" 

#include "software_info.hpp"

#define FORCE_OPTION_DESCRIPTION    "Starts source download and build even if no updates are detected"
#define ABOUT_OPTION_DESCRIPTION    "Displaying software information"
#define CHECK_OPTION_DESCRIPTION    "Checking access of all source's URLs from config"
#define CHILD_OPTION_DESCRIPTION    "Sending release notes to parent proccess (for work in chain)"
#define INIT_OPTION_DESCRIPTION     "Initializing software by creating config and downloading all dependencies"
#define SHOW_OPTION_DESCRIPTION     "Displaying all extra sources from configuration files"
#define ADD_EXTRA_OPTION            "Adding extra source to download list"
#define REMOVE_EXTRA_OPTION         "Removing extra source from download list"

CmdArgs gCmdArgs = { 0 };

CLI::Option* gAddExtraOption;
CLI::Option* gRemoveExtraOption;

void prepareCmdArgs(CLI::App& app, int argc, char** argv) {
    app.description(RGC_DESCRIPTION);

    argv = app.ensure_utf8(argv);

    (void)argv;

    app.add_flag("-f,--force", gCmdArgs.isForceCreation, FORCE_OPTION_DESCRIPTION);
    app.add_flag("--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
    app.add_flag("-c,--check", gCmdArgs.isCheckUrls, CHECK_OPTION_DESCRIPTION);
    app.add_flag("--child", gCmdArgs.isChild, CHILD_OPTION_DESCRIPTION);
    app.add_flag("--init", gCmdArgs.isInit, INIT_OPTION_DESCRIPTION);

    app.add_flag("--show", gCmdArgs.isShowExtras, SHOW_OPTION_DESCRIPTION);

    gAddExtraOption = app.add_option("-a, --add", gCmdArgs.extraSourceId, ADD_EXTRA_OPTION);
    gRemoveExtraOption = app.add_option("-r, --remove", gCmdArgs.extraSourceId, REMOVE_EXTRA_OPTION);
}

bool askYesNo(const std::string& question, bool isYesDefault) {
    std::string userChoice;
    userChoice.reserve(1);

    char yesChar = isYesDefault ? 'Y' : 'y';
    char noChar = isYesDefault ? 'n' : 'N';

    while (true) {
        std::cout << "❔ " << question << " (" << yesChar << "/" << noChar << "): ";
        std::getline(std::cin, userChoice);

        if (userChoice.length() == 0) {
            return isYesDefault;
        } else if (userChoice.length() == 1 && std::tolower(userChoice[0]) == 'y' ||
                   std::tolower(userChoice[0]) == 'n') {

            return std::tolower(userChoice[0]) == 'y';
        }
    }
}
