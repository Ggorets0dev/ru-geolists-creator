#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

#include "main_sources.hpp"

#define GEO_FORMAT_V2RAY_CAPTION    "v2ray"
#define GEO_FORMAT_SING_CAPTION     "sing"

#define IS_FORMAT_REQUESTED(format) \
    (std::find(gCmdArgs.formats.begin(), gCmdArgs.formats.end(), format) != gCmdArgs.formats.end())

struct CmdArgs {
    SourceId extraSourceId;
    std::string outDirPath;
    std::vector<std::string> formats;

    bool isForceCreation;
    bool isUpdateCreation;
    bool isShowAbout;
    bool isCheckUrls;
    bool isChild;
    bool isInit;
    bool isShowExtras;
    bool isAddExtra;
};

extern CmdArgs gCmdArgs;

extern CLI::Option* gRemoveExtraOption;
extern CLI::Option* gOutPathOption;

bool validateParsedFormats();

void printAvailableFormats();

void prepareCmdArgs(CLI::App& app, int argc, char** argv);

bool askYesNo(const std::string& question, bool isYesDefault);

void getStringInput(const std::string& question, std::string& out, bool isEmptyAllowed);

#endif // CLI_ARGS_HPP
