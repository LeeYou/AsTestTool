#include "devices/signature/WindowsSignaturePad.h"
#include "utils/Logger.h"
#include <chrono>

namespace AsTestTool {

WindowsSignaturePad::WindowsSignaturePad(const std::string& type) 
    : m_type(type) {
    LOG_INFO("WindowsSignaturePad created, type: " + type);
    m_libraryLoader = std::make_unique<LibraryLoader>();
}

WindowsSignaturePad::~WindowsSignaturePad() {
    Shutdown();
}

bool WindowsSignaturePad::Initialize() {
    LOG_INFO("Initializing Windows signature pad");
    
    // 尝试加载系统目录下的默认DLL
    std::vector<std::string> defaultPaths = {
        "cmcc_sign.dll",  // 当前目录
        "C:\\Windows\\System32\\cmcc_sign.dll",  // System32目录
        "C:\\Windows\\SysWOW64\\cmcc_sign.dll",  // SysWOW64目录（32位程序）
        "C:\\Program Files\\Common Files\\cmcc_sign.dll",  // 通用文件目录
        "C:\\Program Files (x86)\\Common Files\\cmcc_sign.dll"  // 32位通用文件目录
    };
    
    bool loaded = false;
    for (const auto& path : defaultPaths) {
        if (LoadLibrary(path)) {
            LOG_INFO("Successfully loaded signature pad library from: " + path);
            loaded = true;
            break;
        }
    }
    
    if (!loaded) {
        LOG_WARNING("Failed to load signature pad library from any default location");
        LOG_INFO("You can use 'Load Library' button to select a custom DLL");
    }
    
    m_initialized = true;
    return true;
}

bool WindowsSignaturePad::IsConnected() {
    if (!m_initialized || !m_deviceOpened) {
        return false;
    }
    
    // 检查设备状态
    UINT status = 0;
    int result = GetDeviceInfo(HW_GET_STATUS, &status);
    return (result > 0 && status == 0); // 0表示正常工作
}

void WindowsSignaturePad::Shutdown() {
    LOG_INFO("Shutting down Windows signature pad");
    
    StopCapture();
    CloseDevice();
    
    if (m_libraryLoader && m_libraryLoader->IsLoaded()) {
        m_libraryLoader->UnloadDynamicLibrary();
    }
    
    m_initialized = false;
}

std::string WindowsSignaturePad::GetDeviceInfo() {
    std::string info = "Windows Signature Pad - " + m_type;
    
    if (m_deviceOpened) {
        info += "\nVendor: " + m_vendor;
        info += "\nProduct: " + m_product;
        info += "\nDriver Version: " + std::to_string(HIWORD(m_driverVersion)) + "." + std::to_string(LOWORD(m_driverVersion));
        info += "\nPressure Level: " + std::to_string(m_pressureLevel);
        info += "\nX Axis: " + std::to_string(m_axisX.axMin) + " - " + std::to_string(m_axisX.axMax);
        info += "\nY Axis: " + std::to_string(m_axisY.axMin) + " - " + std::to_string(m_axisY.axMax);
    }
    
    return info;
}

bool WindowsSignaturePad::StartCapture() {
    LOG_INFO("Starting signature capture (Windows implementation)");
    
    if (!m_deviceOpened) {
        LOG_ERROR("Device not opened");
        return false;
    }
    
    if (m_capturing) {
        LOG_WARNING("Already capturing");
        return true;
    }
    
    // 清除之前的数据
    m_signatureData.Clear();
    
    // 启动捕获线程
    m_keepReading = true;
    m_captureThread = std::thread(&WindowsSignaturePad::CaptureLoop, this);
    
    m_capturing = true;
    LOG_INFO("Signature capture started successfully");
    return true;
}

bool WindowsSignaturePad::StopCapture() {
    LOG_INFO("Stopping signature capture (Windows implementation)");
    
    if (!m_capturing) {
        return true;
    }
    
    // 停止捕获线程
    m_keepReading = false;
    if (m_captureThread.joinable()) {
        m_captureThread.join();
    }
    
    m_capturing = false;
    LOG_INFO("Signature capture stopped successfully");
    return true;
}

bool WindowsSignaturePad::GetSignatureData(SignatureData& data) {
    if (!m_capturing) {
        LOG_WARNING("Not currently capturing");
    return false;
    }
    
    data = m_signatureData;
    return data.HasData();
}

bool WindowsSignaturePad::ClearSignature() {
    LOG_INFO("Clearing signature (Windows implementation)");
    m_signatureData.Clear();
    return true;
}

bool WindowsSignaturePad::GetScreenSize(int& width, int& height) {
    if (!m_deviceOpened) {
        return false;
    }
    
    width = m_axisX.axMax - m_axisX.axMin;
    height = m_axisY.axMax - m_axisY.axMin;
    return true;
}

bool WindowsSignaturePad::SetFullscreen(bool fullscreen) {
    LOG_INFO("Setting fullscreen mode: " + std::string(fullscreen ? "true" : "false"));
    m_fullscreen = fullscreen;
    // TODO: 实现实际的全屏模式设置逻辑
    return true;
}

bool WindowsSignaturePad::IsCapturing() {
    return m_capturing;
}

std::vector<std::string> WindowsSignaturePad::GetSupportedPads() {
    return {"default", "cmcc_sign"};
}

// DLL相关方法实现
bool WindowsSignaturePad::LoadLibrary(const std::string& libraryPath) {
    LOG_INFO("Loading signature pad library: " + libraryPath);
    
    if (m_libraryLoader->IsLoaded()) {
        m_libraryLoader->UnloadDynamicLibrary();
    }
    
    if (!m_libraryLoader->LoadDynamicLibrary(libraryPath)) {
        LOG_ERROR("Failed to load library: " + libraryPath);
        return false;
    }
    
    // 解析函数指针
    m_openDeviceFunc = (OpenDeviceFunc)m_libraryLoader->GetFunction("OpenDevice");
    m_getDeviceInfoFunc = (GetDeviceInfoFunc)m_libraryLoader->GetFunction("getDeviceInfo");
    m_getPacketsFunc = (GetPacketsFunc)m_libraryLoader->GetFunction("getPackets");
    m_getGestureFunc = (GetGestureFunc)m_libraryLoader->GetFunction("getGesture");
    
    if (!m_openDeviceFunc || !m_getDeviceInfoFunc || !m_getPacketsFunc) {
        LOG_ERROR("Failed to load required functions from library");
        m_libraryLoader->UnloadDynamicLibrary();
        return false;
    }
    
    LOG_INFO("Library loaded successfully: " + libraryPath);
    return true;
}

bool WindowsSignaturePad::OpenDevice() {
    LOG_INFO("Opening signature pad device");
    
    if (!m_openDeviceFunc) {
        LOG_ERROR("OpenDevice function not loaded");
        return false;
    }
    
    int result = m_openDeviceFunc();
    if (result != SUCCESS) {
        LOG_ERROR("Failed to open device, result: " + std::to_string(result));
        return false;
    }
    
    m_deviceOpened = true;
    
    // 获取设备信息
    if (!ParseDeviceInfo()) {
        LOG_WARNING("Failed to parse device info");
    }
    
    LOG_INFO("Device opened successfully");
    return true;
}

bool WindowsSignaturePad::CloseDevice() {
    LOG_INFO("Closing signature pad device");
    
    if (!m_deviceOpened) {
        return true;
    }
    
    if (m_getDeviceInfoFunc) {
        bool result = false;
        int ret = m_getDeviceInfoFunc(HW_CLOSE, &result);
        if (ret > 0 && result) {
            LOG_INFO("Device closed successfully");
        } else {
            LOG_WARNING("Device close result: " + std::to_string(result));
        }
    }
    
    m_deviceOpened = false;
    return true;
}

int WindowsSignaturePad::GetDeviceInfo(int nIndex, LPVOID lpOutput) {
    if (!m_getDeviceInfoFunc) {
        return 0;
    }
    
    return m_getDeviceInfoFunc(nIndex, lpOutput);
}

int WindowsSignaturePad::GetPackets(PACKETS& packets) {
    if (!m_getPacketsFunc) {
        return -1;
    }
    
    return m_getPacketsFunc(packets);
}

int WindowsSignaturePad::GetGesture(HWND hWnd, UINT uMsg, bool bGesture) {
    if (!m_getGestureFunc) {
        return 0;
    }
    
    return m_getGestureFunc(hWnd, uMsg, bGesture);
}

// 内部方法实现
void WindowsSignaturePad::CaptureLoop() {
    LOG_INFO("Signature capture loop started");
    
    PACKETS packets;
    while (m_keepReading) {
        int result = GetPackets(packets);
        if (result == -1) {
            LOG_WARNING("Device may be disconnected");
            break;
        }
        
        if (result > 0) {
            SignaturePoint point;
            ConvertPacketsToSignaturePoint(packets, point);
            m_signatureData.points.push_back(point);
        }
        
        // 短暂休眠以避免过度占用CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LOG_INFO("Signature capture loop ended");
}

bool WindowsSignaturePad::ParseDeviceInfo() {
    LOG_INFO("Parsing device information");
    
    // 获取X轴范围
    int result = GetDeviceInfo(HW_GET_AXIS_X, &m_axisX);
    if (result <= 0) {
        LOG_WARNING("Failed to get X axis info");
        return false;
    }
    
    // 获取Y轴范围
    result = GetDeviceInfo(HW_GET_AXIS_Y, &m_axisY);
    if (result <= 0) {
        LOG_WARNING("Failed to get Y axis info");
        return false;
    }
    
    // 获取压感级别
    result = GetDeviceInfo(HW_GET_PRESSURE, &m_pressureLevel);
    if (result <= 0) {
        LOG_WARNING("Failed to get pressure level");
    }
    
    // 获取厂商信息
    char vendor[256] = {0};
    result = GetDeviceInfo(HW_GET_VENDOR, vendor);
    if (result > 0) {
        m_vendor = std::string(vendor);
    }
    
    // 获取产品信息
    char product[256] = {0};
    result = GetDeviceInfo(HW_GET_PRODUCT, product);
    if (result > 0) {
        m_product = std::string(product);
    }
    
    // 获取驱动版本
    result = GetDeviceInfo(HW_GET_DRV_VER, &m_driverVersion);
    if (result <= 0) {
        LOG_WARNING("Failed to get driver version");
    }
    
    // 设置手写屏尺寸
    m_signatureData.width = m_axisX.axMax - m_axisX.axMin;
    m_signatureData.height = m_axisY.axMax - m_axisY.axMin;
    m_signatureData.deviceInfo = m_vendor + " " + m_product;
    
    LOG_INFO("Device info parsed successfully");
    LOG_INFO("X Axis: " + std::to_string(m_axisX.axMin) + " - " + std::to_string(m_axisX.axMax));
    LOG_INFO("Y Axis: " + std::to_string(m_axisY.axMin) + " - " + std::to_string(m_axisY.axMax));
    LOG_INFO("Pressure Level: " + std::to_string(m_pressureLevel));
    LOG_INFO("Vendor: " + m_vendor);
    LOG_INFO("Product: " + m_product);
    
    return true;
}

void WindowsSignaturePad::ConvertPacketsToSignaturePoint(const PACKETS& packets, SignaturePoint& point) {
    // 转换坐标到0-1范围
    float x = (float)(packets.X - m_axisX.axMin) / (m_axisX.axMax - m_axisX.axMin);
    float y = (float)(packets.Y - m_axisY.axMin) / (m_axisY.axMax - m_axisY.axMin);
    
    // 转换压力值到0-1范围
    float pressure = (float)packets.Press / m_pressureLevel;
    if (pressure > 1.0f) pressure = 1.0f;
    if (pressure < 0.0f) pressure = 0.0f;
    
    // 判断是否按下（按钮状态或压力值）
    bool isDown = (packets.btn != 0) || (pressure > 0.1f);
    
    // 获取当前时间戳
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    
    point = SignaturePoint(x, y, pressure, timestamp, isDown);
}

} // namespace AsTestTool
