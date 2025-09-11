#include "utils/FileUtils.h"
#include "utils/Logger.h"
#include <fstream>
#include <filesystem>

namespace AsTestTool {

bool FileUtils::FileExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

bool FileUtils::DirectoryExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool FileUtils::CreateDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create directory '" + path + "': " + e.what());
        return false;
    }
}

long long FileUtils::GetFileSize(const std::string& path) {
    try {
        if (FileExists(path)) {
            return std::filesystem::file_size(path);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get file size for '" + path + "': " + e.what());
    }
    return -1;
}

bool FileUtils::ReadFile(const std::string& path, std::string& content) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for reading: " + path);
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        content.resize(size);
        file.read(&content[0], size);
        file.close();

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to read file '" + path + "': " + e.what());
        return false;
    }
}

bool FileUtils::WriteFile(const std::string& path, const std::string& content) {
    try {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for writing: " + path);
            return false;
        }

        file.write(content.c_str(), content.size());
        file.close();

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to write file '" + path + "': " + e.what());
        return false;
    }
}

std::string FileUtils::GetFileExtension(const std::string& path) {
    std::filesystem::path p(path);
    return p.extension().string();
}

std::string FileUtils::GetFileName(const std::string& path) {
    std::filesystem::path p(path);
    return p.filename().string();
}

std::string FileUtils::GetDirectoryPath(const std::string& path) {
    std::filesystem::path p(path);
    return p.parent_path().string();
}

bool FileUtils::ListFiles(const std::string& path, std::vector<std::string>& files) {
    try {
        files.clear();
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to list files in '" + path + "': " + e.what());
        return false;
    }
}

} // namespace AsTestTool
