#include "cli_args.hpp"

CmdArgs gCmdArgs = {
    .isForceCreation = false,
    .isShowHelp = false
};

void
printSoftwareInfo() {
    std::cout << "ru-geolists-creator v" << RGC_VERSION << std::endl;
    std::cout << "Developer: " << RGC_DEVELOPER << std::endl;
    std::cout << "License: " << RGC_LICENSE << std::endl;
    std::cout << "GitHub: " << RGC_REPOSITORY << std::endl;
}

int
parseCmdArgs(int argc, char** argv) {
    CLI::App app{RGC_DESCRIPTION};
    argv = app.ensure_utf8(argv);

    app.add_flag("-f,--force", gCmdArgs.isForceCreation, "Starts source download and build even if no updates are detected");
    app.add_flag("-v,--version", gCmdArgs.isShowHelp, "Displaying software information");

    CLI11_PARSE(app, argc, argv);

    return 0;
}
