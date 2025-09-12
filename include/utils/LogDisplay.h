#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <memory>

namespace AsTestTool {

/**
 * @brief 日志显示级别
 */
enum class LogDisplayLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};

/**
 * @brief 日志条目
 */
struct LogEntry {
    LogDisplayLevel level;
    std::string message;
    std::string timestamp;
    
    LogEntry(LogDisplayLevel l, const std::string& msg, const std::string& ts)
        : level(l), message(msg), timestamp(ts) {}
};

/**
 * @brief 日志显示管理器
 */
class LogDisplay {
public:
    static LogDisplay& Instance();
    
    /**
     * @brief 添加日志条目
     */
    void AddLog(LogDisplayLevel level, const std::string& message);
    
    /**
     * @brief 获取所有日志条目
     */
    const std::vector<LogEntry>& GetLogs() const;
    
    /**
     * @brief 清空日志
     */
    void ClearLogs();
    
    /**
     * @brief 设置最大日志条数
     */
    void SetMaxLogs(size_t maxLogs);
    
    /**
     * @brief 获取当前日志条数
     */
    size_t GetLogCount() const;

private:
    LogDisplay() = default;
    ~LogDisplay() = default;
    LogDisplay(const LogDisplay&) = delete;
    LogDisplay& operator=(const LogDisplay&) = delete;
    
    std::vector<LogEntry> m_logs;
    mutable std::mutex m_mutex;
    size_t m_maxLogs = 1000;
    
    std::string GetCurrentTimestamp() const;
};

} // namespace AsTestTool
