#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace AsTestTool {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};

/**
 * @brief 日志类
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     * @return Logger实例
     */
    static Logger& Instance();

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void SetLevel(LogLevel level);

    /**
     * @brief 设置输出文件
     * @param filename 文件名
     */
    void SetOutputFile(const std::string& filename);

    /**
     * @brief 设置是否输出到控制台
     * @param enable true 输出到控制台，false 不输出
     */
    void SetConsoleOutput(bool enable);

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void Log(LogLevel level, const std::string& message);

    /**
     * @brief 记录调试日志
     * @param message 日志消息
     */
    void Debug(const std::string& message);

    /**
     * @brief 记录信息日志
     * @param message 日志消息
     */
    void Info(const std::string& message);

    /**
     * @brief 记录警告日志
     * @param message 日志消息
     */
    void Warning(const std::string& message);

    /**
     * @brief 记录错误日志
     * @param message 日志消息
     */
    void Error(const std::string& message);

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string GetLevelString(LogLevel level) const;
    std::string GetCurrentTime() const;

    LogLevel m_level = LogLevel::Info;
    std::string m_filename;
    std::ofstream m_file;
    bool m_consoleOutput = true;
    std::mutex m_mutex;
};

// 便捷宏定义
#ifdef ENABLE_LOGGING
#define LOG_DEBUG(msg) AsTestTool::Logger::Instance().Debug(msg)
#define LOG_INFO(msg) AsTestTool::Logger::Instance().Info(msg)
#define LOG_WARNING(msg) AsTestTool::Logger::Instance().Warning(msg)
#define LOG_ERROR(msg) AsTestTool::Logger::Instance().Error(msg)
#else
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARNING(msg)
#define LOG_ERROR(msg)
#endif

} // namespace AsTestTool
