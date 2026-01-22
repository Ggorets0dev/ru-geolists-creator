#ifndef CLI_ARGS_HPP
#define CLI_ARGS_HPP

// Third-party imports (LICENSES are included)
#include "CLI11.hpp"

#include "service.hpp"
#include "main_sources.hpp"

#define GEO_FORMAT_V2RAY_CAPTION    "v2ray"
#define GEO_FORMAT_SING_CAPTION     "sing"

#define DEFAULT_GEOLISTS_OUT_PATH   (fs::current_path() / "rglc_geofiles")

#define IS_FORMAT_REQUESTED(args, format) \
    (std::find(args.formats.begin(), args.formats.end(), format) != args.formats.end())

struct CmdArgs {
    SourceObjectId extraSourceId;
    std::string outDirPath = DEFAULT_GEOLISTS_OUT_PATH;
    std::vector<std::string> formats;
    std::vector<std::string> presets;

    bool isUpdateCreation;
    bool isShowAbout;
    bool isInit;
    bool isAddExtra;
    bool isUseWhitelist;

    // ====== Extra sources show options
    bool isSortBySections;
    bool isSortByStorageTypes;
    bool isSortByeInetTypes;
    // ======s
};

extern CmdArgs gCmdArgs;
extern ServiceSettings gServiceSettings;

extern CLI::App* gBuildSubCmd;
extern CLI::App* gCheckSubCmd;
extern CLI::App* gServiceSubCmd;
extern CLI::App* gShowSubCmd;
extern CLI::Option* gOutPathOption;

bool validateParsedFormats(const CmdArgs& args);

void printAvailableFormats();

void prepareCmdArgs(CLI::App& app);

bool askYesNo(const std::string& question, bool isYesDefault);

void getStringInput(const std::string& question, std::string& out, bool isEmptyAllowed);

#endif // CLI_ARGS_HPP
