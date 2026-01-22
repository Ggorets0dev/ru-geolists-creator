#include "cli_draw.hpp"

#include <iomanip>

void printTableLine(std::ostream& os, const std::vector<int>& widths) {
    os << "+";
    for (const int w : widths) {
        os << std::string(w + 2, '-') << "+";
    }
    os << "\n";
}

void printDoubleRow(std::ostream& os, const std::vector<int>& widths, const std::string& col1, const std::string& col2) {
    os << "| " << std::left << std::setw(widths[0]) << col1 << " | "
       << std::left << std::setw(widths[1]) << col2 << " |\n";
}