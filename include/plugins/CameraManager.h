#pragma once
#include "PluginLoader.h"
#include "ICameraPlugin.h"
#include <memory>
#include <string>

namespace AsTestTool {
namespace Plugins {

class CameraManager {
public:
    CameraManager();
    ~CameraManager();
    
    // 初始化
    bool Initialize();
    void Shutdown();
    
    // 插件管理
    bool LoadPlatformPlugin();
    bool LoadCustomPlugin(const std::string& pluginPath);
    bool UnloadPlugin(const std::string& pluginName);
    
    // 摄像头操作
    std::vector<CameraInfo> GetAvailableCameras();
    bool OpenCamera(const std::string& cameraId);
    bool CloseCamera();
    bool IsCameraOpen() const;
    
    // 预览控制
    bool StartPreview();
    bool StopPreview();
    bool IsPreviewActive() const;
    
    // 图像捕获
    bool CaptureImage(std::vector<uint8_t>& imageData, 
                     PixelFormat& format, 
                     int& width, int& height);
    
    // 设置控制
    bool SetResolution(int width, int height, int fps);
    bool SetPixelFormat(PixelFormat format);
    bool SetBrightness(int value);
    bool SetContrast(int value);
    bool SetSaturation(int value);
    
    // 错误处理
    std::string GetLastError() const;
    int GetLastErrorCode() const;
    
    // 插件信息
    std::vector<std::string> GetLoadedPlugins() const;
    std::string GetCurrentPluginName() const;
    std::string GetCurrentPluginVersion() const;
    std::string GetCurrentPlatform() const;
    bool SwitchPlugin(const std::string& pluginName);
    
    // 配置
    void SetPluginDirectory(const std::string& directory);
    std::string GetPluginDirectory() const;
    
private:
    std::unique_ptr<PluginLoader> m_pluginLoader;
    std::unique_ptr<ICameraPlugin> m_currentPlugin;
    std::string m_currentPluginName;
    std::string m_pluginDirectory;
    std::string m_lastError;
    int m_lastErrorCode;
    bool m_initialized;
    
    // 平台检测
    std::string DetectPlatform() const;
    std::string GetDefaultPluginPath() const;
    std::string GetDefaultPluginName() const;
    
    // 错误处理
    void SetLastError(const std::string& error, int code = -1);
    
    // 插件切换
    bool ValidatePluginSwitch(const std::string& pluginName);
    
    // 辅助方法
    std::string GetExecutablePath() const;
};

} // namespace Plugins
} // namespace AsTestTool
