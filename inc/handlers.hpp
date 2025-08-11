#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <string>
#include <sstream>

#include "log.hpp"
#include "config.hpp"
#include "main_sources.hpp"
#include "build_tools.hpp"
#include "archive.hpp"

extern void
initSoftware();

extern std::tuple<bool, bool>
checkForUpdates(const RgcConfig& config);

extern bool
downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles);

#endif // HANDLERS_HPP
