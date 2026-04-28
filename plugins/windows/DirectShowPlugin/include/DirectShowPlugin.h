#pragma once
#include "../../../../include/plugins/ICameraPlugin.h"
#include <dshow.h>
#include <strmif.h>
#include <vector>
#include <string>
#include <memory>

// 前向声明
struct ISampleGrabber;

namespace AsTestTool {
namespace Plugins {

// 颜色格式枚举
enum class ColorFormat {
    Unknown,
    RGB24,
    BGR24,
    YUV420,
    YUV422,
    MJPG
};

// DirectShow摄像头实现
class DirectShowCamera : public ICameraPlugin {
public:
    DirectShowCamera();
    virtual ~DirectShowCamera();
    
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
    // DirectShow组件
    IGraphBuilder* m_graphBuilder;
    ICaptureGraphBuilder2* m_captureBuilder;
    IBaseFilter* m_cameraFilter;
    ISampleGrabber* m_sampleGrabber;
    IBaseFilter* m_sampleGrabberFilter;
    IMediaControl* m_mediaControl;
    IMediaEventEx* m_mediaEvent;
    IAMStreamConfig* m_streamConfig;
    IAMVideoProcAmp* m_videoProcAmp;
    
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
    
    // 内部方法
    bool InitializeDirectShow();
    void CleanupDirectShow();
    bool EnumerateCameras();
    bool CreateFilterGraph();
    bool ConfigureSampleGrabber();
    bool SetMediaType(int width, int height, int fps, PixelFormat format);
    
    // 错误处理
    void SetLastError(const std::string& error, int code = -1);
    std::string GetDirectShowError(HRESULT hr) const;
    
    // 分辨率管理
    std::vector<Resolution> GetDeviceSupportedResolutions(IMoniker* moniker);
    std::vector<Resolution> GetCommonResolutions();
    std::vector<Resolution> GetDefaultResolutions();
    
    // 像素格式转换
    GUID PixelFormatToGUID(PixelFormat format) const;
    PixelFormat GUIDToPixelFormat(const GUID& guid) const;
    std::string GetFormatName(const GUID& guid) const;
    
    // 真实图像获取
    bool TryGetRealCameraImage(std::vector<uint8_t>& imageData, int width, int height);
    
    // 图像翻转相关方法
    bool ShouldFlipImage() const;
    void FlipImageVertically(const uint8_t* source, uint8_t* destination, int width, int height, size_t dataSize);
    
    // 颜色校正相关方法
    void ApplyColorCorrection(uint8_t* imageData, int width, int height);
    void ConvertBGRToRGB(uint8_t* imageData, int width, int height);
    void ApplyGammaCorrection(uint8_t* imageData, int width, int height, float gamma = 1.0f);
    
    // 智能颜色空间检测
    bool DetectColorFormat();
    ColorFormat GetDetectedColorFormat() const { return m_detectedColorFormat; }
    bool IsColorFormatDetected() const { return m_colorFormatDetected; }
    
    // 自适应颜色校正
    void ApplyAdaptiveColorCorrection(uint8_t* imageData, int width, int height);
    void AnalyzeImageColors(const uint8_t* imageData, int width, int height);
    float CalculateOptimalGamma(const uint8_t* imageData, int width, int height);
    
    // 坏帧检测和处理
    bool IsValidImageData(const uint8_t* data, size_t dataSize, int width, int height);
    bool IsCorruptedFrame(const uint8_t* data, size_t dataSize);
    void HandleBadFrame();
    
    // 公共方法：控制图像翻转
    void SetAutoFlipImage(bool enable) { m_autoFlipImage = enable; }
    bool IsAutoFlipImageEnabled() const { return m_autoFlipImage; }
    
    // 公共方法：控制颜色校正
    void SetAutoColorCorrection(bool enable) { m_autoColorCorrection = enable; }
    bool IsAutoColorCorrectionEnabled() const { return m_autoColorCorrection; }
    void SetBGRToRGBConversion(bool enable) { m_convertBGRToRGB = enable; }
    bool IsBGRToRGBConversionEnabled() const { return m_convertBGRToRGB; }
    void SetGammaValue(float gamma) { m_gammaValue = gamma; }
    float GetGammaValue() const { return m_gammaValue; }
    
    // 公共方法：控制自适应校正
    void SetAdaptiveCorrection(bool enable) { m_adaptiveCorrection = enable; }
    bool IsAdaptiveCorrectionEnabled() const { return m_adaptiveCorrection; }
    void ForceColorFormatDetection() { DetectColorFormat(); }
    std::string GetDetectedColorFormatString() const;
    
    // 公共方法：控制坏帧检测
    void SetBadFrameDetection(bool enable) { m_enableBadFrameDetection = enable; }
    bool IsBadFrameDetectionEnabled() const { return m_enableBadFrameDetection; }
    int GetBadFrameCount() const { return m_badFrameCount; }
    int GetTotalFrameCount() const { return m_totalFrameCount; }
    float GetBadFrameRate() const { return (m_totalFrameCount > 0) ? (float)m_badFrameCount / m_totalFrameCount : 0.0f; }
    void ResetFrameStatistics() { m_badFrameCount = 0; m_totalFrameCount = 0; }
    
    // 摄像头枚举
    std::vector<CameraInfo> m_availableCameras;
    bool m_camerasEnumerated;
    
    // 图像翻转配置
    bool m_autoFlipImage;  // 是否自动翻转图像
    
    // 颜色校正配置
    bool m_autoColorCorrection;  // 是否自动颜色校正
    bool m_convertBGRToRGB;      // 是否转换BGR到RGB
    float m_gammaValue;          // 伽马校正值
    
    // 智能颜色检测
    ColorFormat m_detectedColorFormat;  // 检测到的颜色格式
    bool m_colorFormatDetected;         // 是否已检测颜色格式
    bool m_adaptiveCorrection;          // 是否启用自适应校正
    float m_optimalGamma;               // 计算出的最优伽马值
    
    // 坏帧检测统计
    int m_badFrameCount;                // 坏帧计数
    int m_totalFrameCount;              // 总帧计数
    bool m_enableBadFrameDetection;     // 是否启用坏帧检测
};

// DirectShow插件工厂
class DirectShowFactory : public IPluginFactory {
public:
    DirectShowFactory();
    virtual ~DirectShowFactory();
    
    std::unique_ptr<ICameraPlugin> CreatePlugin() override;
    std::string GetPluginType() const override;
};

} // namespace Plugins
} // namespace AsTestTool

// 导出函数声明
extern "C" {
    __declspec(dllexport) AsTestTool::Plugins::IPluginFactory* CreatePluginFactory();
    __declspec(dllexport) void DestroyPluginFactory(AsTestTool::Plugins::IPluginFactory* factory);
}
