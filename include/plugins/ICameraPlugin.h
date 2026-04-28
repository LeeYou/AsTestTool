#pragma once
#include <string>
#include <vector>
#include <memory>

namespace AsTestTool {
namespace Plugins {

// 分辨率
struct Resolution {
    int width;
    int height;
    int fps;
    std::string description;
    
    Resolution(int w = 0, int h = 0, int f = 30, const std::string& desc = "")
        : width(w), height(h), fps(f), description(desc) {}
};

// 像素格式
enum class PixelFormat {
    YUV420,
    YUV422,
    RGB24,
    BGR24,
    RGB32,
    MJPG,
    H264,
    Unknown
};

// 相机信息结构
struct CameraInfo {
    std::string id;
    std::string name;
    std::string devicePath;
    std::vector<Resolution> supportedResolutions;
    std::vector<PixelFormat> supportedFormats;
    bool isAvailable;
    
    CameraInfo() : isAvailable(false) {}
};

// 相机接口
class ICameraPlugin {
public:
    virtual ~ICameraPlugin() = default;
    
    // 插件信息
    virtual std::string GetPluginName() const = 0;
    virtual std::string GetPluginVersion() const = 0;
    virtual std::string GetPlatform() const = 0;
    
    // 设备管理
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual std::vector<CameraInfo> GetAvailableCameras() = 0;
    
    // 相机管理
    virtual bool OpenCamera(const std::string& cameraId) = 0;
    virtual bool CloseCamera() = 0;
    virtual bool IsCameraOpen() const = 0;
    
    // 预览控制
    virtual bool StartPreview() = 0;
    virtual bool StopPreview() = 0;
    virtual bool IsPreviewActive() const = 0;
    
    // 图像捕获
    virtual bool CaptureImage(std::vector<uint8_t>& imageData, 
                             PixelFormat& format, 
                             int& width, int& height) = 0;
    
    // 设置参数
    virtual bool SetResolution(int width, int height, int fps) = 0;
    virtual bool SetPixelFormat(PixelFormat format) = 0;
    virtual bool SetBrightness(int value) = 0;
    virtual bool SetContrast(int value) = 0;
    virtual bool SetSaturation(int value) = 0;
    
    // 错误信息
    virtual std::string GetLastError() const = 0;
    virtual int GetLastErrorCode() const = 0;
};

// 插件工厂接口
class IPluginFactory {
public:
    virtual ~IPluginFactory() = default;
    virtual std::unique_ptr<ICameraPlugin> CreatePlugin() = 0;
    virtual std::string GetPluginType() const = 0;
};

// External C interface
extern "C" {
    typedef IPluginFactory* (*CreateFactoryFunc)();
    typedef void (*DestroyFactoryFunc)(IPluginFactory*);
}

} // namespace Plugins
} // namespace AsTestTool
