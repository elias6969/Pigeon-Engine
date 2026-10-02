#include <iostream>
#include <fstream>
#include <stdexcept>
#include "filemanager.h"

namespace {
fs::path ResolveBaseDirectory(const std::string &basePath) {
    fs::path requestedPath(basePath);
    if (fs::exists(requestedPath)) {
        return fs::absolute(requestedPath);
    }

#ifdef PIGEON_ASSET_DIR
    fs::path configuredAssets(PIGEON_ASSET_DIR);
    if (fs::exists(configuredAssets) && basePath.find("assets") != std::string::npos) {
        return fs::absolute(configuredAssets);
    }
#endif

    return requestedPath;
}
}

VirtualFileSystem::VirtualFileSystem(const std::string& basePath)
    : baseDir(ResolveBaseDirectory(basePath).string()) {
    if (!fs::exists(baseDir)) {
        std::cerr << "Base directory does not exist: " << fs::absolute(baseDir) << std::endl;
        throw std::runtime_error("Base directory does not exist: " + baseDir);
    }
    std::cout << "BASE-DIRECTORY-FOUND::" << fs::absolute(baseDir) << std::endl;
}


std::string VirtualFileSystem::getFullPath(const std::string& relativePath) const {
    fs::path fullPath = fs::path(baseDir) / relativePath;
    if (!fs::exists(fullPath)) {
        throw std::runtime_error("File not found: " + fullPath.string());
    }
    // Return the absolute path instead of a relative one.
    return fs::absolute(fullPath).string();
}


std::string VirtualFileSystem::readFile(const std::string& relativePath) {
    std::string fullPath = getFullPath(relativePath);
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fullPath);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
