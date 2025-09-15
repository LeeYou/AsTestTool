#include "devices/idcard/WindowsIDCardReader.h"
#include "utils/Logger.h"
#include "utils/StringUtils.h"
#include <cstring>
#include <algorithm>

namespace AsTestTool {

WindowsIDCardReader::WindowsIDCardReader(const std::string& type) 
    : m_type(type), m_libraryLoader(std::make_unique<LibraryLoader>()) {
    LOG_INFO("WindowsIDCardReader created, type: " + type);
}

WindowsIDCardReader::~WindowsIDCardReader() {
    Shutdown();
}

bool WindowsIDCardReader::Initialize() {
    LOG_INFO("Initializing Windows ID card reader");
    
    if (m_initialized) {
        LOG_WARNING("ID card reader already initialized");
        return true;
    }
    
    // 尝试加载系统目录下的默认DLL
    // 使用系统DLL搜索路径，让Windows自动查找
    std::string defaultLibrary = "CMCC_IDCARD.DLL";
    LOG_INFO("Trying to load system library: " + defaultLibrary);
    
    bool loaded = false;
    if (LoadLibrary(defaultLibrary)) {
        LOG_INFO("Successfully loaded ID card reader library from system: " + defaultLibrary);
        loaded = true;
    } else {
        LOG_WARNING("Failed to load system library: " + defaultLibrary);
    }
    
    if (!loaded) {
        LOG_WARNING("Failed to load ID card reader library from any default location");
        LOG_INFO("You can use 'Load Library' button to select a custom DLL");
    }
    
    m_initialized = true;
    LOG_INFO("Windows ID card reader initialized successfully");
    return true;
}

bool WindowsIDCardReader::IsConnected() {
    if (!m_initialized) {
        return false;
    }
    
    if (!m_deviceOpened) {
        return false;
    }
    
    // 检查设备状态
    return CheckDeviceStatus();
}

void WindowsIDCardReader::Shutdown() {
    LOG_INFO("Shutting down Windows ID card reader");
    
    if (m_deviceOpened) {
        CloseDevice();
    }
    
    if (m_libraryLoader) {
        m_libraryLoader->UnloadDynamicLibrary();
    }
    
    m_initialized = false;
    LOG_INFO("Windows ID card reader shutdown complete");
}

std::string WindowsIDCardReader::GetDeviceInfo() {
    std::string info = "Windows ID Card Reader - " + m_type;
    if (m_deviceOpened) {
        info += " (Connected, Port: " + std::to_string(m_devicePort) + ")";
    } else {
        info += " (Not Connected)";
    }
    return info;
}

bool WindowsIDCardReader::ReadCard(IDCardInfo& info) {
    LOG_INFO("Reading ID card (Windows implementation)");
    
    if (!m_initialized) {
        LOG_ERROR("ID card reader not initialized");
        return false;
    }
    
    if (!m_deviceOpened) {
        LOG_ERROR("Device not opened");
        return false;
    }
    
    if (!m_readCardFunc) {
        LOG_ERROR("ReadCard function not loaded");
        return false;
    }
    
    char outMessage[1024] = {0};
    int result = m_readCardFunc(outMessage);
    
    if (result == 0) {
        LOG_INFO("ID card read successfully");
        return ParseCardInfo(info);
    } else {
        LOG_ERROR("Failed to read ID card, error: " + std::string(outMessage));
        return false;
    }
}

std::vector<std::string> WindowsIDCardReader::GetSupportedReaders() {
    return {"default", "cmcc_idcard", "reader1", "reader2"};
}

bool WindowsIDCardReader::HasCard() {
    if (!m_initialized || !m_deviceOpened) {
        return false;
    }
    
    int status = GetDeviceStatus();
    // 根据状态码判断是否有卡片
    // 131000: 正常空闲, 131001: 读取成功
    return (status == 131000 || status == 131001);
}

bool WindowsIDCardReader::EjectCard() {
    LOG_INFO("Ejecting ID card (Windows implementation)");
    
    if (!m_deviceOpened) {
        LOG_ERROR("Device not opened");
        return false;
    }
    
    // 注意：标准接口中没有弹出卡片的函数
    // 这通常由硬件自动处理
    LOG_INFO("Card ejection requested (hardware dependent)");
    return true;
}

bool WindowsIDCardReader::OpenDevice(int port) {
    LOG_INFO("Opening ID card device on port: " + std::to_string(port));
    
    if (!m_initialized) {
        LOG_ERROR("Device not initialized");
        return false;
    }
    
    if (!m_openDeviceFunc) {
        LOG_ERROR("OpenDevice function not loaded - please load library first");
        return false;
    }
    
    if (m_deviceOpened) {
        LOG_WARNING("Device already opened");
        return true;
    }
    
    char outMessage[1024] = {0};
    int result = m_openDeviceFunc(port, outMessage);
    
    if (result == 0) {
        m_deviceOpened = true;
        m_devicePort = port;
        LOG_INFO("Device opened successfully on port: " + std::to_string(port));
        return true;
    } else {
        LOG_ERROR("Failed to open device, error: " + std::string(outMessage));
        return false;
    }
}

bool WindowsIDCardReader::CloseDevice() {
    LOG_INFO("Closing ID card device");
    
    if (!m_deviceOpened) {
        LOG_WARNING("Device not opened");
        return true;
    }
    
    if (!m_closeDeviceFunc) {
        LOG_ERROR("CloseDevice function not loaded");
        return false;
    }
    
    char outMessage[1024] = {0};
    int result = m_closeDeviceFunc(outMessage);
    
    if (result == 0) {
        m_deviceOpened = false;
        LOG_INFO("Device closed successfully");
        return true;
    } else {
        LOG_ERROR("Failed to close device, error: " + std::string(outMessage));
        return false;
    }
}

int WindowsIDCardReader::GetDeviceStatus() {
    if (!m_deviceOpened || !m_getDeviceStatusFunc) {
        return -1;
    }
    
    char outMessage[1024] = {0};
    int status = m_getDeviceStatusFunc(outMessage);
    
    LOG_DEBUG("Device status: " + std::to_string(status) + ", message: " + std::string(outMessage));
    return status;
}

bool WindowsIDCardReader::LoadLibrary(const std::string& libraryPath) {
    LOG_INFO("Loading ID card reader library: " + libraryPath);
    
    if (!m_libraryLoader->LoadDynamicLibrary(libraryPath)) {
        LOG_ERROR("Failed to load library: " + libraryPath);
        return false;
    }
    
    // 加载函数指针
    m_openDeviceFunc = (OpenDeviceFunc)m_libraryLoader->GetFunction("OpenDevice");
    m_readCardFunc = (ReadCardFunc)m_libraryLoader->GetFunction("readCard");
    m_getDeviceStatusFunc = (GetDeviceStatusFunc)m_libraryLoader->GetFunction("getDeviceStatus");
    m_getCardInfoFunc = (GetCardInfoFunc)m_libraryLoader->GetFunction("GetCardInfo");
    m_closeDeviceFunc = (CloseDeviceFunc)m_libraryLoader->GetFunction("CloseDevice");
    
    // 检查关键函数是否加载成功
    if (!m_openDeviceFunc || !m_readCardFunc || !m_getDeviceStatusFunc || 
        !m_getCardInfoFunc || !m_closeDeviceFunc) {
        LOG_ERROR("Failed to load required functions from library");
        m_libraryLoader->UnloadDynamicLibrary();
        return false;
    }
    
    LOG_INFO("Library loaded successfully: " + libraryPath);
    return true;
}

std::string WindowsIDCardReader::GetLastError() const {
    // 这里可以维护一个错误信息缓存
    return "Unknown error";
}

bool WindowsIDCardReader::CheckDeviceStatus() {
    int status = GetDeviceStatus();
    
    // 根据状态码判断设备状态
    switch (status) {
        case 131000: // 正常空闲
        case 131001: // 读取成功
            return true;
        case 431001: // 设备故障
        case 431002: // 端口不存在
        case 431003: // 端口打开失败
        case 431004: // 无响应
        case 431005: // 数据接收异常
        default:
            LOG_WARNING("Device status indicates problem: " + std::to_string(status));
            return false;
    }
}

bool WindowsIDCardReader::ParseCardInfo(IDCardInfo& info) {
    LOG_INFO("Parsing ID card information");
    
    if (!m_getCardInfoFunc) {
        LOG_ERROR("GetCardInfo function not loaded");
        return false;
    }
    
    // 清空现有信息
    info.Clear();
    
    // 获取各个字段的信息
    // Value=0: 姓名, Value=1: 性别, Value=2: 民族, Value=3: 出生日期
    // Value=4: 住址, Value=5: 身份号码, Value=6: 签发机关
    // Value=7: 开始有效期限, Value=8: 结束有效期限
    // Value=10: 照片文件名, Value=11: 照片BASE64编码
    
    char outValue[1024] = {0};
    char outMessage[1024] = {0};
    
    // 获取姓名 (index=0)
    if (m_getCardInfoFunc(0, outValue, outMessage) == 0) {
        info.name = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取性别 (index=1)
    if (m_getCardInfoFunc(1, outValue, outMessage) == 0) {
        info.gender = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取民族 (index=2)
    if (m_getCardInfoFunc(2, outValue, outMessage) == 0) {
        info.nation = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取出生日期 (index=3)
    if (m_getCardInfoFunc(3, outValue, outMessage) == 0) {
        std::string birthDate = StringUtils::Trim(std::string(outValue));
        // 转换YYYYMMDD格式为YYYY-MM-DD
        if (birthDate.length() == 8) {
            info.birthDate = birthDate.substr(0, 4) + "-" + 
                           birthDate.substr(4, 2) + "-" + 
                           birthDate.substr(6, 2);
        } else {
            info.birthDate = birthDate;
        }
    }
    
    // 获取住址 (index=4)
    if (m_getCardInfoFunc(4, outValue, outMessage) == 0) {
        info.address = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取身份证号 (index=5)
    if (m_getCardInfoFunc(5, outValue, outMessage) == 0) {
        info.idNumber = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取签发机关 (index=6)
    if (m_getCardInfoFunc(6, outValue, outMessage) == 0) {
        info.issuingAuthority = StringUtils::Trim(std::string(outValue));
    }
    
    // 获取有效期限 (index=7, 8)
    std::string startDate, endDate;
    if (m_getCardInfoFunc(7, outValue, outMessage) == 0) {
        startDate = StringUtils::Trim(std::string(outValue));
    }
    if (m_getCardInfoFunc(8, outValue, outMessage) == 0) {
        endDate = StringUtils::Trim(std::string(outValue));
    }
    
    // 组合有效期限
    if (!startDate.empty() && !endDate.empty()) {
        info.validPeriod = startDate + " 至 " + endDate;
    } else if (!endDate.empty()) {
        info.validPeriod = endDate;
    }
    
    // 获取照片BASE64编码 (index=11)
    if (m_getCardInfoFunc(11, outValue, outMessage) == 0) {
        std::string photoBase64 = StringUtils::Trim(std::string(outValue));
        if (!photoBase64.empty()) {
            // 这里可以将BASE64转换为二进制数据
            // 暂时存储BASE64字符串
            info.photo.assign(photoBase64.begin(), photoBase64.end());
        }
    }
    
    LOG_INFO("ID card information parsed successfully");
    LOG_INFO("Name: " + info.name);
    LOG_INFO("ID Number: " + info.idNumber);
    LOG_INFO("Gender: " + info.gender);
    LOG_INFO("Birth Date: " + info.birthDate);
    
    return info.IsValid();
}

std::string WindowsIDCardReader::GetCardInfoByIndex(int index) {
    if (!m_getCardInfoFunc) {
        return "";
    }
    
    char outValue[1024] = {0};
    char outMessage[1024] = {0};
    
    if (m_getCardInfoFunc(index, outValue, outMessage) == 0) {
        return StringUtils::Trim(std::string(outValue));
    }
    
    return "";
}

} // namespace AsTestTool
