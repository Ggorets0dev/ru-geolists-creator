#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "cli_args.hpp"
#include "config.hpp"
#include "main_sources.hpp"

void showExtraSources();

void initSoftware();

void deinitSoftware();

void addExtraSource();

void removeExtraSource(SourceId id);

std::tuple<bool, bool> checkForUpdates(const RgcConfig& config);

bool downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles);

void printHelp(const CLI::App& app);

void printSoftwareInfo();

void checkUrlsAccess();

#endif // HANDLERS_HPP
