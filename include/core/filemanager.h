
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class VirtualFileSystem {
public:
    explicit VirtualFileSystem(const std::string& basePath);
    std::string getFullPath(const std::string& relativePath) const;
    std::string readFile(const std::string& relativePath);

private:
    std::string baseDir;
};

#endif // FILEMANAGER_H

