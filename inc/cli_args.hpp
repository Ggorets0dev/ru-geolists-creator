#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

struct CmdArgs {
    bool isForceCreation;
    bool isShowAbout;
    bool isCheckUrls;
    bool isChild;
    bool isInit;
};

extern CmdArgs gCmdArgs;

void prepareCmdArgs(CLI::App& app, int argc, char** argv);

bool askYesNo(const std::string& question, bool isYesDefault);

#endif // CLI_ARGS_HPP
