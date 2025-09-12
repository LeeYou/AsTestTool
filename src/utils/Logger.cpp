#include "utils/Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

// 前向声明，避免在插件中依赖LogDisplay
#ifdef ENABLE_LOG_DISPLAY
#include "utils/LogDisplay.h"
#endif

namespace AsTestTool {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::SetLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

void Logger::SetOutputFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_filename = filename;
    if (m_file.is_open()) {
        m_file.close();
    }
    if (!filename.empty()) {
        m_file.open(filename, std::ios::app);
    }
}

void Logger::SetConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < m_level) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << "[" << GetCurrentTime() << "] "
       << "[" << GetLevelString(level) << "] "
       << message << std::endl;

    std::string logMessage = ss.str();

    // 输出到控制台
    if (m_consoleOutput) {
        if (level >= LogLevel::Error) {
            std::cerr << logMessage;
        } else {
            std::cout << logMessage;
        }
    }

    // 输出到文件
    if (m_file.is_open()) {
        m_file << logMessage;
        m_file.flush();
    }
    
    // 调用日志回调（如果设置了）
    if (m_logCallback) {
        m_logCallback(level, message);
    }
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::SetLogCallback(std::function<void(LogLevel, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logCallback = callback;
}

std::string Logger::GetLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::GetCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
#ifdef _WIN32
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t);
    ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
#else
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

} // namespace AsTestTool
