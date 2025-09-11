#include "core/Config.h"
#include "utils/Logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace AsTestTool {

Config& Config::Instance() {
    static Config instance;
    return instance;
}

bool Config::LoadFromFile(const std::string& filename) {
    LOG_INFO("Loading config from file: " + filename);
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_WARNING("Failed to open config file: " + filename);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行和注释行
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // 查找等号
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // 去除前后空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            m_config[key] = value;
        }
    }

    file.close();
    LOG_INFO("Config loaded successfully, " + std::to_string(m_config.size()) + " entries");
    return true;
}

bool Config::SaveToFile(const std::string& filename) {
    LOG_INFO("Saving config to file: " + filename);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR("Failed to create config file: " + filename);
        return false;
    }

    file << "# AsTestTool Configuration File\n";
    file << "# Generated automatically\n\n";

    for (const auto& pair : m_config) {
        file << pair.first << "=" << pair.second << "\n";
    }

    file.close();
    LOG_INFO("Config saved successfully");
    return true;
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        return it->second;
    }
    return defaultValue;
}

void Config::SetString(const std::string& key, const std::string& value) {
    m_config[key] = value;
}

int Config::GetInt(const std::string& key, int defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            LOG_WARNING("Failed to parse int value for key '" + key + "': " + e.what());
        }
    }
    return defaultValue;
}

void Config::SetInt(const std::string& key, int value) {
    m_config[key] = std::to_string(value);
}

bool Config::GetBool(const std::string& key, bool defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value == "true" || value == "1" || value == "yes";
    }
    return defaultValue;
}

void Config::SetBool(const std::string& key, bool value) {
    m_config[key] = value ? "true" : "false";
}

double Config::GetDouble(const std::string& key, double defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception& e) {
            LOG_WARNING("Failed to parse double value for key '" + key + "': " + e.what());
        }
    }
    return defaultValue;
}

void Config::SetDouble(const std::string& key, double value) {
    m_config[key] = std::to_string(value);
}

bool Config::HasKey(const std::string& key) const {
    return m_config.find(key) != m_config.end();
}

void Config::RemoveKey(const std::string& key) {
    m_config.erase(key);
}

void Config::Clear() {
    m_config.clear();
}

std::vector<std::string> Config::GetAllKeys() const {
    std::vector<std::string> keys;
    for (const auto& pair : m_config) {
        keys.push_back(pair.first);
    }
    return keys;
}

void Config::SetDefaults() {
    LOG_INFO("Setting default configuration");
    
    // 应用程序配置
    SetString("app.name", "AsTestTool");
    SetString("app.version", "1.0.0");
    SetString("app.theme", "light");
    
    // 日志配置
    SetString("log.level", "info");
    SetString("log.file", "astesttool.log");
    SetBool("log.console", true);
    
    // 身份证阅读器配置
    SetString("idcard.default_library", "");
    SetString("idcard.library_path", "libs/");
    
    // 摄像头配置
    SetInt("camera.default_width", 1920);
    SetInt("camera.default_height", 1080);
    SetInt("camera.default_fps", 30);
    SetInt("camera.brightness", 50);
    SetInt("camera.contrast", 50);
    
    // 手写屏配置
    SetString("signature.default_library", "");
    SetString("signature.library_path", "libs/");
    SetBool("signature.fullscreen", true);
    
    // 窗口配置
    SetInt("window.width", 1200);
    SetInt("window.height", 800);
    SetBool("window.maximized", false);
}

} // namespace AsTestTool
