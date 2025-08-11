#include "archive.hpp"
#include "log.hpp"

#include <fstream>
#include <archive.h>
#include <archive_entry.h>

#define ARCHIVE_BUFFER_BYTES    10240u // 10 kb
#define NEW_DIR_ACCESS_CODE     0755

std::optional<std::string>
extractTarGz(const std::string& archivePath, const std::string& destDirPath) {
    struct archive *a;
    struct archive_entry *entry;
    std::string rootDirPath;
    int r;

    // Открываем архив для чтения
    a = archive_read_new();
    if (a == nullptr) {
        return std::nullopt;
    }

    // Устанавливаем формат чтения .tar
    archive_read_support_format_tar(a);
    archive_read_support_compression_gzip(a);

    r = archive_read_open_filename(a, archivePath.c_str(), ARCHIVE_BUFFER_BYTES);
    if (r != ARCHIVE_OK) {
        log(LogType::ERROR, "Failed to open archive for read:", archive_error_string(a));
        return std::nullopt;
    }

    // Распаковываем файлы из архива
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char* filename = archive_entry_pathname(entry);
        std::string fullPath = destDirPath + "/" + filename;

        if (rootDirPath.empty()) {
            rootDirPath = fullPath;
        }

        // Создаем директории, если их нет
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            mkdir(fullPath.c_str(), NEW_DIR_ACCESS_CODE);
        } else {
            std::ofstream outFile(fullPath, std::ios::binary);
            if (!outFile) {
                LOG_ERROR(FILE_OPEN_ERROR_MSG + fullPath);
                return std::nullopt;
            }

            // Читаем содержимое архива и записываем в файл
            const void* buffer;
            size_t size;
            int64_t offset;

            while (archive_read_data_block(a, &buffer, &size, &offset) == ARCHIVE_OK) {
                outFile.write(static_cast<const char*>(buffer), size);
            }
            outFile.close();
        }
        archive_read_data_skip(a);  // Пропускаем данные этого файла
    }

    if (r != ARCHIVE_EOF) {
        log(LogType::ERROR, "Failed to read archive: ", archive_error_string(a));
    }

    archive_read_free(a);  // Освобождаем ресурсы

    LOG_INFO("Successfully extracted the archive to the path: " + rootDirPath);

    return rootDirPath;
}
