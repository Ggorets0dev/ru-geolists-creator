#include "ruadlist.hpp"

#define MAX_DOMAIN_SIZE         56
#define DOMAIN_REGEX            R"((?:[a-zA-Z0-9-]+\.)+(?:by|ru|ua|info|sex|su|life|com|ml|best|net|site|biz|xyz))"

#define START_SEGMENT_MARK      "! *** advblock/first_level.txt ***"
#define STOP_SEGMENT_MARK       "! *** advblock/specific_hide.txt ***"

bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    std::ifstream inputFile(inputFilePath); // Открываем файл

    if (!inputFile.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + inputFilePath);
        return false;
    }

    std::ofstream outputFile(outputFilePath); // Открываем файл

    if (!outputFile.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + outputFilePath);
        return false;
    }

    std::string currentDomain;
    std::string lastFoundDomain = "";

    currentDomain.reserve(MAX_DOMAIN_SIZE);
    lastFoundDomain.reserve(MAX_DOMAIN_SIZE);

    std::string line;
    std::regex domainRegex(DOMAIN_REGEX);
    std::smatch match;

    bool segmentBlock = true;

    while (std::getline(inputFile, line)) { // Читаем файл построчно
        if (segmentBlock && line == START_SEGMENT_MARK) {
            segmentBlock = false;
        } else if (line == STOP_SEGMENT_MARK) {
            inputFile.close();
            outputFile.close();

            break;
        }

        if (segmentBlock) {
            continue;
        }

        // Итераторы для поиска совпадений
        auto begin = std::sregex_iterator(line.begin(), line.end(), domainRegex);
        auto end = std::sregex_iterator();

        // Вывод всех найденных доменов
        for (auto it = begin; it != end; ++it) {
            match = *it;

            currentDomain = match.str();

            if (currentDomain == lastFoundDomain || checkKeywordBlacklist(currentDomain)) {
                continue;
            }

            std::cout << currentDomain << std::endl;
            outputFile << currentDomain << std::endl;

            lastFoundDomain = std::move(currentDomain);
        }
    }

    inputFile.close();
    outputFile.close();

    return true;
}
