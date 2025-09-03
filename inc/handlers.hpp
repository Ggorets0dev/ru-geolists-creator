#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "config.hpp"
#include "main_sources.hpp"

void showExtraSources();

void initSoftware();

void deinitSoftware();

void addExtraSource();

void removeExtraSource(SourceId id);

std::tuple<bool, bool> checkForUpdates(const RgcConfig& config);

bool downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles);

void printSoftwareInfo();

void checkUrlsAccess();

#endif // HANDLERS_HPP
