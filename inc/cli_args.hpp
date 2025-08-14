#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

#include <iostream>

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

#include "software_info.hpp"

struct CmdArgs {
    bool isForceCreation;
    bool isShowAbout;
    bool isCheckUrls;
};

extern CmdArgs gCmdArgs;

extern void
printSoftwareInfo();

extern void
checkUrlsAccess();

extern void
prepareCmdArgs(CLI::App& app, int argc, char** argv);

#endif // CLI_ARGS_HPP
