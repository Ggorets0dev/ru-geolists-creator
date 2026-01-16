#include "cli_draw.hpp"

void printTableLine(std::ostream& os, const std::vector<int>& widths) {
    os << "+";
    for (const int w : widths) {
        os << std::string(w + 2, '-') << "+";
    }
    os << "\n";
}