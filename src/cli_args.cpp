#include "cli_args.hpp" 

#define FORCE_OPTION_DESCRIPTION    "Starts source download and build even if no updates are detected"
#define ABOUT_OPTION_DESCRIPTION    "Displaying software information"

CmdArgs gCmdArgs = {
    .isForceCreation = false,
    .isShowAbout = false
};

void
printSoftwareInfo() {
    std::cout << "ru-geolists-creator v" << RGC_VERSION << std::endl;
    std::cout << "Developer: " << RGC_DEVELOPER << std::endl;
    std::cout << "License: " << RGC_LICENSE << std::endl;
    std::cout << "GitHub: " << RGC_REPOSITORY << std::endl;
}

void
prepareCmdArgs(CLI::App& app, int argc, char** argv) {
    app.description(RGC_DESCRIPTION);

    argv = app.ensure_utf8(argv);

    app.add_flag("-f,--force", gCmdArgs.isForceCreation, FORCE_OPTION_DESCRIPTION);
    app.add_flag("-a,--about", gCmdArgs.isShowAbout, ABOUT_OPTION_DESCRIPTION);
}
