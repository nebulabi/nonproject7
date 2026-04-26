#include "FileSystem.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace Amuse {

std::string FileSystem::getAppDirectory() {
    return fs::current_path().string();
}

std::string FileSystem::getModelsDirectory() {
    return getAppDirectory() + "/models";
}

std::string FileSystem::getConfigDirectory() {
    #ifdef _WIN32
        const char* appData = std::getenv("APPDATA");
        if (appData) {
            return std::string(appData) + "/AmuseUnchained";
        }
    #else
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.config/AmuseUnchained";
        }
    #endif
    return getAppDirectory() + "/config";
}

std::string FileSystem::getOutputDirectory() {
    return getAppDirectory() + "/output";
}

bool FileSystem::ensureDirectoryExists(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            std::cout << "[FileSystem] Created directory: " << path << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[FileSystem] Error creating directory " << path 
                  << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::fileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

size_t FileSystem::getFileSize(const std::string& path) {
    if (!fileExists(path)) {
        return 0;
    }
    return fs::file_size(path);
}

std::string FileSystem::getFileExtension(const std::string& path) {
    return fs::path(path).extension().string();
}

std::string FileSystem::getFileNameWithoutExtension(const std::string& path) {
    return fs::path(path).stem().string();
}

std::vector<std::string> FileSystem::listFilesWithExtension(
    const std::string& directory, 
    const std::string& extension
) {
    std::vector<std::string> files;
    
    if (!fs::exists(directory)) {
        return files;
    }
    
    std::string targetExt = extension;
    std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext == targetExt) {
                files.push_back(entry.path().string());
            }
        }
    }
    
    return files;
}

} // namespace Amuse
