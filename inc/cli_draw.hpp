#ifndef CLI_DRAW_HPP
#define CLI_DRAW_HPP

#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <fmt/format.h>
#include <algorithm>

class TablePrinter {
public:
    explicit TablePrinter(std::vector<std::string> headers) : headers(std::move(headers)) {
        columnWidths.resize(this->headers.size());
        for (size_t i = 0; i < this->headers.size(); ++i) {
            updateWidth(i, this->headers[i]);
        }
    }

    void addRow(std::vector<std::string> row) {
        if (row.size() != headers.size()) {
            row.resize(headers.size(), "");
        }
        for (size_t i = 0; i < row.size(); ++i) {
            updateWidth(i, row[i]);
        }
        rows.push_back(std::move(row));
    }

    void print(std::ostream& out) const {
        const std::string separator = makeSeparator();

        out << separator << "\n";
        printRow(out, headers);
        out << separator << "\n";

        for (const auto& row : rows) {
            printRow(out, row);
        }
        out << separator << "\n";
    }

private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::vector<size_t> columnWidths;

    void updateWidth(size_t col, const std::string& text) {
        columnWidths[col] = std::max(columnWidths[col], text.length());
    }

    [[nodiscard]] std::string makeSeparator() const {
        std::string sep = "+";
        for (auto width : columnWidths) {
            sep += std::string(width + 2, '-') + "+";
        }
        return sep;
    }

    void printRow(std::ostream& out, const std::vector<std::string>& row) const {
        out << "|";
        for (size_t i = 0; i < row.size(); ++i) {
            out << fmt::format(" {:<{}} |", row[i], columnWidths[i]);
        }
        out << "\n";
    }
};

#endif // CLI_DRAW_HPP