#pragma once

#include <vector>
#include <string>
#include <memory>
#include "plugins/CameraManager.h"

// ImGui forward declarations
struct ImVec2;

namespace AsTestTool {

/**
 * @brief 摄像头面板类
 */
class CameraPanel {
public:
    CameraPanel();
    ~CameraPanel();

    /**
     * @brief 渲染面板
     */
    void Render();

    /**
     * @brief 设置摄像头设备
     */
    void SetCameraManager(std::shared_ptr<Plugins::CameraManager> manager);

    /**
     * @brief 处理键盘快捷键
     */
    void HandleKeyboardShortcuts();

private:
    // 布局相关
    enum class LayoutMode {
        Compact,    // 紧凑布局
        Standard,   // 标准布局
        Wide        // 宽屏布局
    };

    // 拍摄模式
    enum class CaptureMode {
        Normal,     // 普通模式
        Document,   // 文档模式
        IDCard,     // 身份证模式
        A4Document  // A4文档模式
    };

    // 渲染方法
    void RenderDeviceSelection();
    void RenderDeviceControls();
    void RenderPreviewArea();
    void RenderControlPanel();
    void RenderImageSettings();
    void RenderDocumentMode();
    void RenderImageEnhancement();
    void RenderPreviewOverlay();
    void RenderImageInfo();
    void RenderGridLines(const ImVec2& startPos, const ImVec2& size);
    void RenderResolutionSelector();

    // 功能方法
    void UpdateLayout();
    ImVec2 CalculatePreviewSize(const ImVec2& availableSize, const Plugins::Resolution& resolution);
    void CaptureImage();
    void ToggleRecording();
    void ToggleFullscreen();
    void ExitFullscreen();
    void RefreshDeviceList();
    void DetectDocumentEdges();
    void StartPreview();
    void StopPreview();

    // 成员变量
    std::shared_ptr<Plugins::CameraManager> m_cameraManager;
    bool m_connected = false;
    bool m_previewActive = false;
    bool m_recording = false;
    bool m_fullscreen = false;
    
    LayoutMode m_layoutMode = LayoutMode::Standard;
    CaptureMode m_captureMode = CaptureMode::Normal;
    
    Plugins::Resolution m_currentResolution = Plugins::Resolution(1920, 1080);
    int m_currentFPS = 30;
    std::string m_currentFormat = "MJPEG";
    
    // 图像参数
    int m_brightness = 50;
    int m_contrast = 50;
    int m_saturation = 50;
    int m_sharpness = 50;
    int m_denoise = 30;
    bool m_autoEnhance = true;
    bool m_autoRotate = true;
    
    // 设备列表
    std::vector<Plugins::CameraInfo> m_availableCameras;
    int m_selectedDevice = 0;
    
    // 预览纹理
    void* m_previewTexture = nullptr;
    
    // 图像数据
    std::vector<uint8_t> m_imageData;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    bool m_hasImageData = false;
    
    // 文档检测
    float m_edgeThreshold = 0.5f;
    bool m_showGridLines = false;
};

} // namespace AsTestTool
