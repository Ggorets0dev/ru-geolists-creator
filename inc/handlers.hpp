#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>

#include "log.hpp"
#include "config.hpp"
#include "main_sources.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "ruadlist.hpp"
#include "archive.hpp"

namespace fs = std::filesystem;

extern void
initSoftware();

extern std::tuple<bool, bool>
checkForUpdates();
