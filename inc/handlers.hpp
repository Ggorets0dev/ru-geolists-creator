#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "cli_args.hpp"
#include "config.hpp"
#include "main_sources.hpp"
#include "print_about_handler.hpp"
#include "download_sources_handler.hpp"

void showExtraSources();

void initSoftware();

void deinitSoftware();

void addExtraSource();

void removeExtraSource(SourceId id);

std::tuple<bool, bool> checkForUpdates(const RgcConfig& config);

void printHelp(const CLI::App& app);

void checkUrlsAccess();

#endif // HANDLERS_HPP
