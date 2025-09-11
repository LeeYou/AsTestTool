#include "core/ErrorCode.h"
#include "utils/Logger.h"

namespace AsTestTool {

AsTestToolException::AsTestToolException(ErrorCode code, const std::string& message)
    : m_code(code), m_message(message) {
    LOG_ERROR("Exception: " + GetErrorString() + " - " + message);
}

std::string AsTestToolException::GetErrorString() const {
    return ErrorCodeToString(m_code);
}

std::string ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::DeviceNotFound:
            return "Device not found";
        case ErrorCode::DeviceNotConnected:
            return "Device not connected";
        case ErrorCode::InvalidParameter:
            return "Invalid parameter";
        case ErrorCode::LibraryLoadFailed:
            return "Library load failed";
        case ErrorCode::FunctionNotFound:
            return "Function not found";
        case ErrorCode::PlatformNotSupported:
            return "Platform not supported";
        case ErrorCode::InitializationFailed:
            return "Initialization failed";
        case ErrorCode::OperationFailed:
            return "Operation failed";
        case ErrorCode::Timeout:
            return "Timeout";
        case ErrorCode::Unknown:
        default:
            return "Unknown error";
    }
}

} // namespace AsTestTool
