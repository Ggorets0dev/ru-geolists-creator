#include <iostream>
#include <fstream>
#include <json/json.h>

#include "network.hpp"

int main() {
    downloadFile("https://easylist-downloads.adblockplus.org/ruadlist+easylist.txt", "test.txt");
    downloadFile("https://api.github.com/repos/1andrevich/Re-filter-lists/releases/latest", "req.json");

    // Открываем файл для чтения
    std::ifstream file("req.json", std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: data.json" << std::endl;
        return 1;
    }

    // Парсим JSON из файла
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, file, &root, &errs)) {
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
        return 1;
    }

    file.close();

    std::cout << "Name: " << root["url"].asString() << std::endl;

    return 0;
}
