#include "fs_utils_temp.hpp"

#include "common.hpp"

using namespace FS::Utils;

fs::path Temp::getSessionTempDir() {
    // Global controller for all software launch
    static const SessionTempDirController controller(getSystemTempDir());
    return controller.getTempDir();
}

fs::path Temp::getSystemTempDir() {
    const char* tmpDir = std::getenv("TMPDIR");
    return tmpDir ? fs::path(tmpDir) : fs::path("/tmp");
}

Temp::SessionTempFileRegistry::~SessionTempFileRegistry() {
    for (const auto& file : m_sessionTempFiles) {
        if (fs::exists(file->path)) {
            fs::remove(file->path);
        }
    }
}

Temp::SessionTempFileRegistry::TempFilePtrW Temp::SessionTempFileRegistry::createTempFile(const std::string& ext) {
    const auto salt = genRandomDigits(m_saltSize);

    const auto file = std::make_shared<SessionTempFile>();
    file->path = buildFilePath(ext);

    // Reg file for future delete
    m_sessionTempFiles.push_front(file);

    // Create file if needed...

    return file;
}

Temp::SessionTempFileRegistry::TempFilePtrS Temp::SessionTempFileRegistry::createTempFileDetached(const std::string& ext) const {
    const auto salt = genRandomDigits(m_saltSize);

    const auto file = std::make_shared<SessionTempFile>();
    file->path = buildFilePath(ext);

    // Create file if needed...

    return file;
}

void Temp::SessionTempFileRegistry::deleteTempFile(std::weak_ptr<Temp::SessionTempFile> file) {
    for (auto iter = m_sessionTempFiles.begin(); iter != m_sessionTempFiles.end(); ++iter) {
        if (file.lock() == *iter) {
            if (fs::exists((*iter)->path)) {
                fs::remove((*iter)->path);
            }

            m_sessionTempFiles.erase_after(iter);
            return; // Avoiding UB
        }
    }
}

Temp::SessionTempDirController::SessionTempDirController(const fs::path& dir) {
    m_tempDir = dir / ("rglc_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
}

Temp::SessionTempDirController::~SessionTempDirController() {
    if (fs::exists(m_tempDir)) {
        fs::remove_all(m_tempDir);
    }
}