#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include "cli_args.hpp"
#include "config.hpp"
#include "main_sources.hpp"
#include "print_about_handler.hpp"
#include "build_lists_handler.hpp"

void showPresets(const CmdArgs& args);

void initSoftware();

void deinitSoftware();

void printHelp(const CLI::App& app);

void checkUrlsAccess(const CmdArgs& args);

#endif // HANDLERS_HPP
