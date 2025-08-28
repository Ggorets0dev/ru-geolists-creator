#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

struct CmdArgs {
    bool isForceCreation;
    bool isShowAbout;
    bool isCheckUrls;
    bool isChild;
};

extern CmdArgs gCmdArgs;

extern void
printSoftwareInfo();

extern void
checkUrlsAccess();

extern void
prepareCmdArgs(CLI::App& app, int argc, char** argv);

#endif // CLI_ARGS_HPP
