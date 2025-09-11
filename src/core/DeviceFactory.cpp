#include "core/DeviceFactory.h"
#include "utils/Logger.h"
#include "core/ErrorCode.h"

// 平台特定的实现
#ifdef PLATFORM_WINDOWS
#include "devices/idcard/WindowsIDCardReader.h"
#include "devices/camera/WindowsCamera.h"
#include "devices/signature/WindowsSignaturePad.h"
#elif PLATFORM_LINUX
#include "devices/idcard/LinuxIDCardReader.h"
#include "devices/camera/LinuxCamera.h"
#include "devices/signature/LinuxSignaturePad.h"
#endif

namespace AsTestTool {

std::unique_ptr<IIDCardReader> DeviceFactory::CreateIDCardReader(const std::string& type) {
    LOG_INFO("Creating ID card reader, type: " + (type.empty() ? "default" : type));
    
    try {
#ifdef PLATFORM_WINDOWS
        return std::make_unique<WindowsIDCardReader>(type);
#elif PLATFORM_LINUX
        return std::make_unique<LinuxIDCardReader>(type);
#else
        throw AsTestToolException(ErrorCode::PlatformNotSupported, 
                                "Platform not supported for ID card reader");
#endif
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create ID card reader: " + std::string(e.what()));
        return nullptr;
    }
}

std::unique_ptr<ICamera> DeviceFactory::CreateCamera() {
    LOG_INFO("Creating camera");
    
    try {
#ifdef PLATFORM_WINDOWS
        return std::make_unique<WindowsCamera>();
#elif PLATFORM_LINUX
        return std::make_unique<LinuxCamera>();
#else
        throw AsTestToolException(ErrorCode::PlatformNotSupported, 
                                "Platform not supported for camera");
#endif
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create camera: " + std::string(e.what()));
        return nullptr;
    }
}

std::unique_ptr<ISignaturePad> DeviceFactory::CreateSignaturePad(const std::string& type) {
    LOG_INFO("Creating signature pad, type: " + (type.empty() ? "default" : type));
    
    try {
#ifdef PLATFORM_WINDOWS
        return std::make_unique<WindowsSignaturePad>(type);
#elif PLATFORM_LINUX
        return std::make_unique<LinuxSignaturePad>(type);
#else
        throw AsTestToolException(ErrorCode::PlatformNotSupported, 
                                "Platform not supported for signature pad");
#endif
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create signature pad: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::string> DeviceFactory::GetSupportedIDCardReaders() {
    std::vector<std::string> readers;
    
#ifdef PLATFORM_WINDOWS
    // Windows平台支持的身份证阅读器
    readers.push_back("default");
    readers.push_back("reader1");
    readers.push_back("reader2");
#elif PLATFORM_LINUX
    // Linux平台支持的身份证阅读器
    readers.push_back("default");
    readers.push_back("reader1");
    readers.push_back("reader2");
#endif
    
    return readers;
}

std::vector<std::string> DeviceFactory::GetSupportedSignaturePads() {
    std::vector<std::string> pads;
    
#ifdef PLATFORM_WINDOWS
    // Windows平台支持的手写屏
    pads.push_back("default");
    pads.push_back("pad1");
    pads.push_back("pad2");
#elif PLATFORM_LINUX
    // Linux平台支持的手写屏
    pads.push_back("default");
    pads.push_back("pad1");
    pads.push_back("pad2");
#endif
    
    return pads;
}

} // namespace AsTestTool
