#include "build_tools.hpp"
#include "time_tools.hpp"
#include "cli_args.hpp"
#include "cli_draw.hpp"
#include "log.hpp"
#include "software_info.hpp"

#include <sys/stat.h>
#include <csignal>

#include <ctime>
#include <iomanip>
#include <vector>
#include <fstream>

bool setBuildInfoToRelNotes(std::ofstream& file) {
    if (!file.is_open()) return false;

    const std::time_t now = std::time(nullptr);
    const std::tm* utc_tm = std::gmtime(&now);

    if (!utc_tm) return false;

    char timeBuffer[25];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%d.%m.%Y %H:%M:%S", utc_tm);

    const std::string softwareString = fmt::format("RGLC v{}", RGC_VERSION_STRING);

    const std::vector widths = {20, 25};

    file << "Release Technical Information\n";

    printTableLine(file, widths);
    printDoubleRow(file, widths, "Parameter", "Value");
    printTableLine(file, widths);

    printDoubleRow(file, widths, "Build DateTime (UTC)", timeBuffer);
    printDoubleRow(file, widths, "Software", softwareString);

    printTableLine(file, widths);

    return true;
}

bool addPresetToRelNotes(std::ofstream& file, const SourcePreset& preset) {
    preset.print(file, SourcePreset::SORT_BY_ID);
    return true;
}
