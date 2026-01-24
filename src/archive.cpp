#include "archive.hpp"
#include "log.hpp"
#include "fs_utils.hpp"

#include <fstream>
#include <archive.h>
#include <archive_entry.h>

#define ARCHIVE_BUFFER_BYTES    10240u // 10 kb
#define NEW_DIR_ACCESS_CODE     0755

std::optional<std::string> extractTarGz(const std::string& archivePath, const std::string& destDirPath) {
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
        LOG_ERROR("Failed to open archive for read:" + std::string(archive_error_string(a)));
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

            if (!outFile.is_open()) {
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
        LOG_ERROR("Failed to read archive: " + std::string(archive_error_string(a)));
    }

    archive_read_free(a);  // Освобождаем ресурсы

    if (rootDirPath[rootDirPath.size() - 1] == '/') {
        rootDirPath.pop_back();
    }

    LOG_INFO("Successfully extracted the archive to the path: " + rootDirPath);

    return rootDirPath;
}

std::optional<std::string> createZipArchive(const std::string& folderPath, const std::string& archiveName) {
    fs::path targetDir(folderPath);
    if (!fs::exists(targetDir)) {
        LOG_ERROR("Target path does not exist: {}", folderPath);
        return std::nullopt;
    }

    std::string outputFilename = (targetDir / (archiveName + ".zip")).string();
    LOG_INFO("Starting compression (ZIP): {}", outputFilename);

    struct archive* a = archive_write_new();
    archive_write_set_format_zip(a);

    if (archive_write_open_filename(a, outputFilename.c_str()) != ARCHIVE_OK) {
        LOG_ERROR("Failed to open output file: {}", archive_error_string(a));
        archive_write_free(a);
        return std::nullopt;
    }

    for (const auto& dirEntry : fs::recursive_directory_iterator(folderPath)) {
        std::string currentPath = dirEntry.path().string();

        // Skip the archive itself to avoid recursion
        if (currentPath == outputFilename) continue;

        // Skip directories for data writing, focus on files
        if (fs::is_directory(dirEntry.path())) continue;

        std::string relativePath = fs::relative(dirEntry.path(), folderPath).string();

        struct archive_entry* entry = archive_entry_new();
        archive_entry_set_pathname(entry, relativePath.c_str());
        archive_entry_set_size(entry, fs::file_size(dirEntry.path()));
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);

        if (archive_write_header(a, entry) < ARCHIVE_OK) {
            LOG_WARNING("Could not write header for {}: {}", relativePath, archive_error_string(a));
            archive_entry_free(entry);
            continue;
        }

        std::ifstream fileStream(currentPath, std::ios::binary);
        char buffer[8192];
        while (fileStream.read(buffer, sizeof(buffer)) || fileStream.gcount() > 0) {
            if (archive_write_data(a, buffer, fileStream.gcount()) < 0) {
                LOG_ERROR("Error writing data for {}: {}", relativePath, archive_error_string(a));
                break;
            }
        }

        archive_entry_free(entry);
    }

    archive_write_close(a);
    archive_write_free(a);

    LOG_INFO("Archive (ZIP) created successfully at: {}", outputFilename);

    return outputFilename;
}