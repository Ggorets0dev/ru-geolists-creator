#ifndef CLI_DRAW_HPP
#define CLI_DRAW_HPP

#include <sstream>
#include <vector>

void printTableLine(std::ostream& os, const std::vector<int>& widths);

void printDoubleRow(std::ostream& os, const std::vector<int>& widths, const std::string& col1, const std::string& col2);

#endif // CLI_DRAW_HPP