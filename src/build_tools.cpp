#include "build_tools.hpp"
#include "time_tools.hpp"
#include "cli_args.hpp"
#include "cli_draw.hpp"
#include "log.hpp"
#include "software_info.hpp"

#include <sys/stat.h>
#include <csignal>

#include <ctime>
#include <vector>
#include <fstream>

bool setBuildInfoToRelNotes(std::ofstream& file) {
    if (!file.is_open()) {
        return false;
    }

    const std::time_t now = std::time(nullptr);
    const std::tm* utcTime = std::gmtime(&now);

    if (!utcTime) {
        return false;
    }

    char timeBuffer[25];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%d.%m.%Y %H:%M:%S", utcTime);

    const std::string softwareString = fmt::format("RGLC v{}", RGC_VERSION_STRING);

    file << "Release Technical Information\n";

    TablePrinter table({"Parameter", "Value"});
    table.addRow({"Build DateTime (UTC)", timeBuffer});
    table.addRow({"Software", softwareString});

    table.print(file);

    return true;
}

bool addPresetToRelNotes(std::ofstream& file, const SourcePreset& preset) {
    if (!file.is_open()) {
        return false;
    }

    preset.print(file, SourcePreset::SORT_BY_ID);

    return true;
}
