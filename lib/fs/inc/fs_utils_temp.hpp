#ifndef FS_UTILS_TEMP_HPP
#define FS_UTILS_TEMP_HPP

#include "fs_utils_types_base.hpp"

#include <string>
#include <forward_list>

#include "../../common/inc/common.hpp"

namespace FS::Utils::Temp {
    fs::path getSystemTempDir();

    fs::path getSessionTempDir();

    struct SessionTempFile {
        fs::path path;
    };

    class SessionTempFileRegistry {
    public:
        using TempFilePtrW = std::weak_ptr<SessionTempFile>;
        using TempFilePtrS = std::shared_ptr<SessionTempFile>;

        static constexpr size_t kDefaultSaltSize = 10;

        TempFilePtrW createTempFile(const std::string& ext);
        TempFilePtrS createTempFileDetached(const std::string& ext) const;

        void deleteTempFile(std::weak_ptr<SessionTempFile> file);

        fs::path getTempDir() const { return m_dir; };

        explicit SessionTempFileRegistry(const fs::path& dir=getSessionTempDir(), const std::string& prefix = "noprefix", const size_t saltSize=kDefaultSaltSize) :
            m_dir(dir), m_prefix(prefix), m_saltSize(saltSize) {}

        ~SessionTempFileRegistry();

    private:
        std::forward_list<std::shared_ptr<SessionTempFile>> m_sessionTempFiles;
        fs::path m_dir;
        std::string m_prefix;
        size_t m_saltSize;

        std::string buildFilePath(const std::string& ext) const {
            return m_dir.string() + "/" + m_prefix + "_" + genRandomDigits(m_saltSize) + "." + ext;
        }
    };

    class SessionTempDirController {
    public:
        SessionTempDirController(const fs::path& dir);
        ~SessionTempDirController();

        fs::path getTempDir() const { return m_tempDir; };

    private:
        fs::path m_dir;
        fs::path m_tempDir;
    };
}

#endif //FS_UTILS_TEMP_HPP