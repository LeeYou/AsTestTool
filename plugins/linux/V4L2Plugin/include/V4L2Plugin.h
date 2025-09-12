#pragma once
#include "../../../../include/plugins/ICameraPlugin.h"
#include <linux/videodev2.h>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace AsTestTool {
namespace Plugins {

// V4L2摄像头实现
class V4L2Camera : public ICameraPlugin {
public:
    V4L2Camera();
    virtual ~V4L2Camera();
    
    // 插件信息
    std::string GetPluginName() const override;
    std::string GetPluginVersion() const override;
    std::string GetPlatform() const override;
    
    // 设备管理
    bool Initialize() override;
    void Shutdown() override;
    std::vector<CameraInfo> GetAvailableCameras() override;
    
    // 摄像头操作
    bool OpenCamera(const std::string& cameraId) override;
    bool CloseCamera() override;
    bool IsCameraOpen() const override;
    
    // 预览控制
    bool StartPreview() override;
    bool StopPreview() override;
    bool IsPreviewActive() const override;
    
    // 图像捕获
    bool CaptureImage(std::vector<uint8_t>& imageData, 
                     PixelFormat& format, 
                     int& width, int& height) override;
    
    // 设置控制
    bool SetResolution(int width, int height, int fps) override;
    bool SetPixelFormat(PixelFormat format) override;
    bool SetBrightness(int value) override;
    bool SetContrast(int value) override;
    bool SetSaturation(int value) override;
    
    // 错误处理
    std::string GetLastError() const override;
    int GetLastErrorCode() const override;
    
private:
    // V4L2设备
    int m_deviceFd;
    std::string m_devicePath;
    
    // 状态管理
    bool m_initialized;
    bool m_cameraOpen;
    bool m_previewActive;
    std::string m_currentCameraId;
    std::string m_lastError;
    int m_lastErrorCode;
    
    // 当前设置
    int m_currentWidth;
    int m_currentHeight;
    int m_currentFps;
    PixelFormat m_currentFormat;
    
    // 缓冲区管理
    struct Buffer {
        void* start;
        size_t length;
    };
    std::vector<Buffer> m_buffers;
    int m_bufferCount;
    
    // 线程管理
    std::thread m_captureThread;
    std::atomic<bool> m_captureRunning;
    std::mutex m_captureMutex;
    
    // 帧数据管理
    std::mutex m_frameMutex;
    int m_latestFrameIndex;
    size_t m_latestFrameSize;
    bool m_frameAvailable;
    
    // 内部方法
    bool InitializeV4L2();
    void CleanupV4L2();
    bool EnumerateCameras();
    bool OpenDevice(const std::string& devicePath);
    bool CloseDevice();
    bool SetupDevice();
    bool AllocateBuffers();
    void FreeBuffers();
    bool StartStreaming();
    bool StopStreaming();
    void CaptureLoop();
    
    // 错误处理
    void SetLastError(const std::string& error, int code = -1);
    std::string GetV4L2Error(int errorCode) const;
    
    // 像素格式转换
    uint32_t PixelFormatToV4L2(PixelFormat format) const;
    PixelFormat V4L2ToPixelFormat(uint32_t v4l2Format) const;
    std::string V4L2FormatToString(uint32_t format) const;
    
    // 图像数据处理
    void ConvertToRGB(uint8_t* inputData, uint8_t* rgbData, int width, int height, PixelFormat format);
    void ConvertYUV420ToRGB(uint8_t* yuvData, uint8_t* rgbData, int width, int height);
    void ConvertYUV422ToRGB(uint8_t* yuvData, uint8_t* rgbData, int width, int height);
    void ConvertRGB24ToRGB24(uint8_t* inputData, uint8_t* rgbData, int width, int height);
    void ConvertRGB32ToRGB24(uint8_t* rgb32Data, uint8_t* rgb24Data, int width, int height);
    void GenerateTestPattern(uint8_t* rgbData, int width, int height);
    void GenerateRealisticCameraImage(uint8_t* rgbData, int width, int height);
    
    // 格式检测
    bool AutoDetectBestFormat();
    bool TryFormat(PixelFormat format);
    
    // 摄像头枚举
    std::vector<CameraInfo> m_availableCameras;
    bool m_camerasEnumerated;
    
    // 设备能力检查
    bool CheckDeviceCapabilities(int fd);
    std::vector<Resolution> GetSupportedResolutions(int fd, uint32_t pixelFormat);
    std::vector<PixelFormat> GetSupportedPixelFormats(int fd);
};

// V4L2插件工厂
class V4L2Factory : public IPluginFactory {
public:
    V4L2Factory();
    virtual ~V4L2Factory();
    
    std::unique_ptr<ICameraPlugin> CreatePlugin() override;
    std::string GetPluginType() const override;
};

} // namespace Plugins
} // namespace AsTestTool

// 导出函数声明
extern "C" {
    AsTestTool::Plugins::IPluginFactory* CreatePluginFactory();
    void DestroyPluginFactory(AsTestTool::Plugins::IPluginFactory* factory);
}
