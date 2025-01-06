#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

#include <iostream>

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

#include "software_info.hpp"

struct CmdArgs {
    bool isForceCreation;
    bool isShowHelp;
};

extern CmdArgs gCmdArgs;

extern void
printSoftwareInfo();

extern int
parseCmdArgs(int argc, char** argv);

#endif // CLI_ARGS_HPP
