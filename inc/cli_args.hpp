#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

#include "main_sources.hpp"

struct CmdArgs {
    SourceId extraSourceId;

    bool isForceCreation;
    bool isShowAbout;
    bool isCheckUrls;
    bool isChild;
    bool isInit;
    bool isShowExtras;
    bool isAddExtra;
};

extern CmdArgs gCmdArgs;

extern CLI::Option* gRemoveExtraOption;

void prepareCmdArgs(CLI::App& app, int argc, char** argv);

bool askYesNo(const std::string& question, bool isYesDefault);

void getStringInput(const std::string& question, std::string& out, bool isEmptyAllowed);

#endif // CLI_ARGS_HPP
