#include <filesystem>

#include "log.hpp"
#include "config.hpp"
#include "main_sources.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "ruadlist.hpp"
#include "archive.hpp"

namespace fs = std::filesystem;

using DownloadedSourcePair = std::pair<Source, fs::path>;

extern void
initSoftware();

extern std::tuple<bool, bool>
checkForUpdates(const RgcConfig& config);

extern bool
downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles);
