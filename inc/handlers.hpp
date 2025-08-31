#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "config.hpp"
#include "main_sources.hpp"

extern void
initSoftware();

extern std::tuple<bool, bool>
checkForUpdates(const RgcConfig& config);

extern bool
downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles);

#endif // HANDLERS_HPP
