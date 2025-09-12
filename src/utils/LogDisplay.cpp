#include "utils/LogDisplay.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace AsTestTool {

LogDisplay& LogDisplay::Instance() {
    static LogDisplay instance;
    return instance;
}

void LogDisplay::AddLog(LogDisplayLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string timestamp = GetCurrentTimestamp();
    m_logs.emplace_back(level, message, timestamp);
    
    // 限制日志条数
    if (m_logs.size() > m_maxLogs) {
        m_logs.erase(m_logs.begin(), m_logs.begin() + (m_logs.size() - m_maxLogs));
    }
}

const std::vector<LogEntry>& LogDisplay::GetLogs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logs;
}

void LogDisplay::ClearLogs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.clear();
}

void LogDisplay::SetMaxLogs(size_t maxLogs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxLogs = maxLogs;
}

size_t LogDisplay::GetLogCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logs.size();
}

std::string LogDisplay::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

} // namespace AsTestTool
