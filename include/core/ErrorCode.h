#pragma once

#include <string>
#include <exception>

namespace AsTestTool {

/**
 * @brief 错误代码枚举
 */
enum class ErrorCode {
    Success = 0,
    DeviceNotFound,
    DeviceNotConnected,
    InvalidParameter,
    LibraryLoadFailed,
    FunctionNotFound,
    PlatformNotSupported,
    InitializationFailed,
    OperationFailed,
    Timeout,
    Unknown
};

/**
 * @brief 异常类
 */
class AsTestToolException : public std::exception {
public:
    AsTestToolException(ErrorCode code, const std::string& message);
    
    ErrorCode GetErrorCode() const { return m_code; }
    const char* what() const noexcept override { return m_message.c_str(); }
    
    /**
     * @brief 获取错误代码的字符串描述
     * @return 错误描述
     */
    std::string GetErrorString() const;

private:
    ErrorCode m_code;
    std::string m_message;
};

/**
 * @brief 错误代码转换为字符串
 * @param code 错误代码
 * @return 错误描述字符串
 */
std::string ErrorCodeToString(ErrorCode code);

} // namespace AsTestTool
