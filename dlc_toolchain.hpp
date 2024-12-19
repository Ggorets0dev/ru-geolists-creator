#ifndef DLC_TOOLCHAIN_HPP
#define DLC_TOOLCHAIN_HPP

#include <thread>
#include <chrono>
#include <filesystem>

#include "json_io.hpp"
#include "network.hpp"

bool
downloadDlcSourceCode();

bool
clearDlcDataSection(std::string dlcRootPath);

#endif // DLC_TOOLCHAIN_HPP
