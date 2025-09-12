#include "plugins/CameraManager.h"
#include "utils/Logger.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace AsTestTool {
namespace Plugins {

CameraManager::CameraManager() 
    : m_pluginLoader(std::make_unique<PluginLoader>())
    , m_lastErrorCode(0)
    , m_initialized(false) {
}

CameraManager::~CameraManager() {
    Shutdown();
}

bool CameraManager::Initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // 设置默认插件目录
        if (m_pluginDirectory.empty()) {
            // 获取可执行文件所在目录
            std::string exePath = GetExecutablePath();
            std::string exeDir = exePath.substr(0, exePath.find_last_of("/\\"));
            m_pluginDirectory = exeDir + "/plugins";
        }
        
        // 自动发现并加载平台插件
        if (!LoadPlatformPlugin()) {
            SetLastError("Failed to load platform plugin", -1);
            return false;
        }
        
        m_initialized = true;
        Logger::Instance().Log(LogLevel::Info, "CameraManager initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception during initialization: " + std::string(e.what()), -2);
        return false;
    }
}

void CameraManager::Shutdown() {
    if (!m_initialized) {
        return;
    }
    
    try {
        // 关闭当前摄像头
        if (m_currentPlugin && IsCameraOpen()) {
            CloseCamera();
        }
        
        // 清理当前插件
        m_currentPlugin.reset();
        m_currentPluginName.clear();
        
        // 卸载所有插件
        if (m_pluginLoader) {
            m_pluginLoader->UnloadAllPlugins();
        }
        
        m_initialized = false;
        Logger::Instance().Log(LogLevel::Info, "CameraManager shutdown completed");
        
    } catch (const std::exception& e) {
        Logger::Instance().Log(LogLevel::Error, "Exception during shutdown: " + std::string(e.what()));
    }
}

bool CameraManager::LoadPlatformPlugin() {
    try {
        std::string platform = DetectPlatform();
        std::string pluginName = GetDefaultPluginName();
        std::string pluginPath = GetDefaultPluginPath();
        
        Logger::Instance().Log(LogLevel::Info, "Detected platform: " + platform);
        Logger::Instance().Log(LogLevel::Info, "Loading plugin: " + pluginName + " from " + pluginPath);
        
        // 检查插件文件是否存在
        if (!std::filesystem::exists(pluginPath)) {
            SetLastError("Platform plugin not found: " + pluginPath, -3);
            return false;
        }
        
        // 加载插件
        if (!m_pluginLoader->LoadPlugin(pluginPath)) {
            SetLastError("Failed to load platform plugin: " + m_pluginLoader->GetLastError(), -4);
            return false;
        }
        
        // 创建插件实例
        m_currentPlugin = m_pluginLoader->CreatePlugin(pluginName);
        if (!m_currentPlugin) {
            SetLastError("Failed to create plugin instance: " + m_pluginLoader->GetLastError(), -5);
            return false;
        }
        
        // 初始化插件
        if (!m_currentPlugin->Initialize()) {
            SetLastError("Failed to initialize plugin: " + m_currentPlugin->GetLastError(), -6);
            m_currentPlugin.reset();
            return false;
        }
        
        m_currentPluginName = pluginName;
        Logger::Instance().Log(LogLevel::Info, "Successfully loaded and initialized plugin: " + pluginName);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while loading platform plugin: " + std::string(e.what()), -7);
        return false;
    }
}

bool CameraManager::LoadCustomPlugin(const std::string& pluginPath) {
    try {
        if (!m_pluginLoader) {
            SetLastError("PluginLoader not initialized", -8);
            return false;
        }
        
        // 加载插件
        if (!m_pluginLoader->LoadPlugin(pluginPath)) {
            SetLastError("Failed to load custom plugin: " + m_pluginLoader->GetLastError(), -9);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully loaded custom plugin: " + pluginPath);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while loading custom plugin: " + std::string(e.what()), -10);
        return false;
    }
}

bool CameraManager::UnloadPlugin(const std::string& pluginName) {
    try {
        if (!m_pluginLoader) {
            SetLastError("PluginLoader not initialized", -11);
            return false;
        }
        
        // 如果正在使用该插件，先关闭摄像头
        if (m_currentPluginName == pluginName && IsCameraOpen()) {
            CloseCamera();
        }
        
        // 如果正在使用该插件，清理当前插件
        if (m_currentPluginName == pluginName) {
            m_currentPlugin.reset();
            m_currentPluginName.clear();
        }
        
        // 卸载插件
        if (!m_pluginLoader->UnloadPlugin(pluginName)) {
            SetLastError("Failed to unload plugin: " + m_pluginLoader->GetLastError(), -12);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully unloaded plugin: " + pluginName);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while unloading plugin: " + std::string(e.what()), -13);
        return false;
    }
}

std::vector<CameraInfo> CameraManager::GetAvailableCameras() {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -14);
        return {};
    }
    
    try {
        return m_currentPlugin->GetAvailableCameras();
    } catch (const std::exception& e) {
        SetLastError("Exception while getting available cameras: " + std::string(e.what()), -15);
        return {};
    }
}

bool CameraManager::OpenCamera(const std::string& cameraId) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -16);
        return false;
    }
    
    try {
        Logger::Instance().Log(LogLevel::Info, "CameraManager::OpenCamera called with ID: " + cameraId);
        
        // 如果已经有摄像头打开，先关闭
        if (IsCameraOpen()) {
            CloseCamera();
        }
        
        Logger::Instance().Log(LogLevel::Info, "Calling plugin OpenCamera method...");
        if (!m_currentPlugin->OpenCamera(cameraId)) {
            std::string pluginError = m_currentPlugin->GetLastError();
            Logger::Instance().Log(LogLevel::Error, "Plugin OpenCamera failed, error: '" + pluginError + "'");
            SetLastError("Failed to open camera: " + pluginError, -17);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully opened camera: " + cameraId);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while opening camera: " + std::string(e.what()), -18);
        return false;
    }
}

bool CameraManager::CloseCamera() {
    if (!m_currentPlugin) {
        return true; // 没有插件，认为已经关闭
    }
    
    try {
        if (IsCameraOpen()) {
            if (!m_currentPlugin->CloseCamera()) {
                SetLastError("Failed to close camera: " + m_currentPlugin->GetLastError(), -19);
                return false;
            }
            Logger::Instance().Log(LogLevel::Info, "Successfully closed camera");
        }
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while closing camera: " + std::string(e.what()), -20);
        return false;
    }
}

bool CameraManager::IsCameraOpen() const {
    if (!m_currentPlugin) {
        return false;
    }
    
    try {
        return m_currentPlugin->IsCameraOpen();
    } catch (const std::exception& e) {
        Logger::Instance().Log(LogLevel::Error, "Exception while checking camera status: " + std::string(e.what()));
        return false;
    }
}

bool CameraManager::StartPreview() {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -21);
        return false;
    }
    
    try {
        if (!m_currentPlugin->StartPreview()) {
            SetLastError("Failed to start preview: " + m_currentPlugin->GetLastError(), -22);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully started preview");
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while starting preview: " + std::string(e.what()), -23);
        return false;
    }
}

bool CameraManager::StopPreview() {
    if (!m_currentPlugin) {
        return true; // 没有插件，认为已经停止
    }
    
    try {
        if (IsPreviewActive()) {
            if (!m_currentPlugin->StopPreview()) {
                SetLastError("Failed to stop preview: " + m_currentPlugin->GetLastError(), -24);
                return false;
            }
            Logger::Instance().Log(LogLevel::Info, "Successfully stopped preview");
        }
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while stopping preview: " + std::string(e.what()), -25);
        return false;
    }
}

bool CameraManager::IsPreviewActive() const {
    if (!m_currentPlugin) {
        return false;
    }
    
    try {
        return m_currentPlugin->IsPreviewActive();
    } catch (const std::exception& e) {
        Logger::Instance().Log(LogLevel::Error, "Exception while checking preview status: " + std::string(e.what()));
        return false;
    }
}

bool CameraManager::CaptureImage(std::vector<uint8_t>& imageData, 
                                PixelFormat& format, 
                                int& width, int& height) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -26);
        return false;
    }
    
    try {
        if (!m_currentPlugin->CaptureImage(imageData, format, width, height)) {
            SetLastError("Failed to capture image: " + m_currentPlugin->GetLastError(), -27);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully captured image: " + 
                   std::to_string(width) + "x" + std::to_string(height));
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while capturing image: " + std::string(e.what()), -28);
        return false;
    }
}

bool CameraManager::SetResolution(int width, int height, int fps) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -29);
        return false;
    }
    
    try {
        if (!m_currentPlugin->SetResolution(width, height, fps)) {
            SetLastError("Failed to set resolution: " + m_currentPlugin->GetLastError(), -30);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully set resolution: " + 
                   std::to_string(width) + "x" + std::to_string(height) + "@" + std::to_string(fps));
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting resolution: " + std::string(e.what()), -31);
        return false;
    }
}

bool CameraManager::SetPixelFormat(PixelFormat format) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -32);
        return false;
    }
    
    try {
        if (!m_currentPlugin->SetPixelFormat(format)) {
            SetLastError("Failed to set pixel format: " + m_currentPlugin->GetLastError(), -33);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully set pixel format");
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting pixel format: " + std::string(e.what()), -34);
        return false;
    }
}

bool CameraManager::SetBrightness(int value) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -35);
        return false;
    }
    
    try {
        if (!m_currentPlugin->SetBrightness(value)) {
            SetLastError("Failed to set brightness: " + m_currentPlugin->GetLastError(), -36);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully set brightness: " + std::to_string(value));
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting brightness: " + std::string(e.what()), -37);
        return false;
    }
}

bool CameraManager::SetContrast(int value) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -38);
        return false;
    }
    
    try {
        if (!m_currentPlugin->SetContrast(value)) {
            SetLastError("Failed to set contrast: " + m_currentPlugin->GetLastError(), -39);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully set contrast: " + std::to_string(value));
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting contrast: " + std::string(e.what()), -40);
        return false;
    }
}

bool CameraManager::SetSaturation(int value) {
    if (!m_currentPlugin) {
        SetLastError("No plugin loaded", -41);
        return false;
    }
    
    try {
        if (!m_currentPlugin->SetSaturation(value)) {
            SetLastError("Failed to set saturation: " + m_currentPlugin->GetLastError(), -42);
            return false;
        }
        
        Logger::Instance().Log(LogLevel::Info, "Successfully set saturation: " + std::to_string(value));
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting saturation: " + std::string(e.what()), -43);
        return false;
    }
}

std::string CameraManager::GetLastError() const {
    return m_lastError;
}

int CameraManager::GetLastErrorCode() const {
    return m_lastErrorCode;
}

std::vector<std::string> CameraManager::GetLoadedPlugins() const {
    if (!m_pluginLoader) {
        return {};
    }
    return m_pluginLoader->GetLoadedPlugins();
}

std::string CameraManager::GetCurrentPluginName() const {
    return m_currentPluginName;
}

std::string CameraManager::GetCurrentPluginVersion() const {
    if (!m_currentPlugin) {
        return "";
    }
    return m_currentPlugin->GetPluginVersion();
}

std::string CameraManager::GetCurrentPlatform() const {
    if (!m_currentPlugin) {
        return "";
    }
    return m_currentPlugin->GetPlatform();
}

bool CameraManager::SwitchPlugin(const std::string& pluginName) {
    try {
        if (!ValidatePluginSwitch(pluginName)) {
            return false;
        }
        
        // 关闭当前摄像头
        if (IsCameraOpen()) {
            CloseCamera();
        }
        
        // 创建新的插件实例
        auto newPlugin = m_pluginLoader->CreatePlugin(pluginName);
        if (!newPlugin) {
            SetLastError("Failed to create plugin instance: " + m_pluginLoader->GetLastError(), -44);
            return false;
        }
        
        // 初始化新插件
        if (!newPlugin->Initialize()) {
            SetLastError("Failed to initialize plugin: " + newPlugin->GetLastError(), -45);
            return false;
        }
        
        // 切换插件
        m_currentPlugin = std::move(newPlugin);
        m_currentPluginName = pluginName;
        
        Logger::Instance().Log(LogLevel::Info, "Successfully switched to plugin: " + pluginName);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while switching plugin: " + std::string(e.what()), -46);
        return false;
    }
}

void CameraManager::SetPluginDirectory(const std::string& directory) {
    m_pluginDirectory = directory;
}

std::string CameraManager::GetPluginDirectory() const {
    return m_pluginDirectory;
}

std::string CameraManager::DetectPlatform() const {
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#else
    return "Unknown";
#endif
}

std::string CameraManager::GetDefaultPluginPath() const {
    std::string platform = DetectPlatform();
    std::string pluginName = GetDefaultPluginName();
    
    if (platform == "Windows") {
        return m_pluginDirectory + "/windows/" + pluginName + ".dll";
    } else if (platform == "Linux") {
        return m_pluginDirectory + "/linux/" + pluginName + ".so";
    }
    
    return "";
}

std::string CameraManager::GetDefaultPluginName() const {
    std::string platform = DetectPlatform();
    
    if (platform == "Windows") {
        return "DirectShowPlugin";
    } else if (platform == "Linux") {
        return "V4L2Plugin";
    }
    
    return "";
}

void CameraManager::SetLastError(const std::string& error, int code) {
    m_lastError = error;
    m_lastErrorCode = code;
    Logger::Instance().Log(LogLevel::Error, "CameraManager Error [" + std::to_string(code) + "]: " + error);
}

bool CameraManager::ValidatePluginSwitch(const std::string& pluginName) {
    if (!m_pluginLoader) {
        SetLastError("PluginLoader not initialized", -47);
        return false;
    }
    
    if (!m_pluginLoader->IsPluginLoaded(pluginName)) {
        SetLastError("Plugin not loaded: " + pluginName, -48);
        return false;
    }
    
    return true;
}

std::string CameraManager::GetExecutablePath() const {
#ifdef _WIN32
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (length > 0) {
        return std::string(path);
    }
#else
    char path[1024];
    ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length > 0) {
        path[length] = '\0';
        return std::string(path);
    }
#endif
    return "";
}

} // namespace Plugins
} // namespace AsTestTool
