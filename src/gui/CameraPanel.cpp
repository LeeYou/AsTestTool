#include "gui/CameraPanel.h"
#include "plugins/CameraManager.h"
#include "utils/Logger.h"
#include "imgui.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace AsTestTool {

CameraPanel::CameraPanel() {
    LOG_INFO("CameraPanel created");
    // 初始化默认分辨率
    m_currentResolution = Plugins::Resolution(1920, 1080);
}

CameraPanel::~CameraPanel() {
    LOG_INFO("CameraPanel destroyed");
    if (m_cameraManager) {
        StopPreview();
    }
}

void CameraPanel::Render() {
    UpdateLayout();
    HandleKeyboardShortcuts();
    
    // 适应新的面板尺寸 - 垂直布局
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    // 设备选择和控制区域 - 顶部
    ImGui::BeginChild("DeviceControls", ImVec2(-1, 120), true);
    RenderDeviceControls();
    ImGui::EndChild();
    
    // 预览区域 - 中间主要区域
    ImGui::BeginChild("PreviewArea", ImVec2(-1, availableSize.y - 180), true, ImGuiWindowFlags_NoScrollbar);
    RenderPreviewArea();
    ImGui::EndChild();
    
    // 图像信息 - 底部
    ImGui::BeginChild("ImageInfo", ImVec2(-1, 60), true);
    RenderImageInfo();
    ImGui::EndChild();
}

void CameraPanel::SetCameraManager(std::shared_ptr<Plugins::CameraManager> manager) {
    LOG_INFO("Setting camera manager...");
    m_cameraManager = manager;
    if (m_cameraManager) {
        LOG_INFO("Camera manager set successfully");
        m_connected = m_cameraManager->IsCameraOpen();
        m_availableCameras = m_cameraManager->GetAvailableCameras();
        LOG_INFO("Found " + std::to_string(m_availableCameras.size()) + " available cameras");
        
        // 刷新设备列表
        RefreshDeviceList();
    } else {
        LOG_ERROR("Failed to set camera manager - manager is null");
    }
}

void CameraPanel::HandleKeyboardShortcuts() {
    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        CaptureImage();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
        ToggleFullscreen();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) && m_fullscreen) {
        ExitFullscreen();
    }
}

void CameraPanel::RenderDeviceSelection() {
    ImGui::Text("设备选择");
    ImGui::SameLine();
    
    // 设备下拉选择
    if (ImGui::BeginCombo("##device", m_availableCameras.empty() ? "无设备" : m_availableCameras[m_selectedDevice].name.c_str())) {
        for (int i = 0; i < m_availableCameras.size(); i++) {
            bool isSelected = (m_selectedDevice == i);
            if (ImGui::Selectable(m_availableCameras[i].name.c_str(), isSelected)) {
                m_selectedDevice = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    
    // 连接状态指示
    ImVec4 statusColor = m_connected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImGui::TextColored(statusColor, m_connected ? "● 已连接" : "● 未连接");
    
    // 换行显示操作按钮
    ImGui::NewLine();
    ImGui::Text("操作:");
    ImGui::SameLine();
    
    // 刷新设备列表按钮
    if (ImGui::Button("刷新设备", ImVec2(80, 25))) {
        RefreshDeviceList();
    }
    
    ImGui::SameLine();
    
    // 连接/断开按钮
    if (m_connected) {
        if (ImGui::Button("断开连接", ImVec2(80, 25))) {
            LOG_INFO("Disconnecting camera...");
            StopPreview();
            if (m_cameraManager) {
                m_cameraManager->CloseCamera();
            }
            m_connected = false;
            LOG_INFO("Camera disconnected");
        }
    } else {
        if (ImGui::Button("连接设备", ImVec2(80, 25))) {
            LOG_INFO("Connecting to camera...");
            if (m_cameraManager && m_cameraManager->Initialize()) {
                // 获取可用摄像头
                auto cameras = m_cameraManager->GetAvailableCameras();
                if (!cameras.empty() && m_selectedDevice < cameras.size()) {
                    // 打开选中的摄像头
                    if (m_cameraManager->OpenCamera(cameras[m_selectedDevice].id)) {
                        m_connected = true;
                        LOG_INFO("Camera connected successfully: " + cameras[m_selectedDevice].name);
                        StartPreview();
                    } else {
                        LOG_ERROR("Failed to open camera: " + cameras[m_selectedDevice].name);
                    }
                } else {
                    LOG_ERROR("No camera selected or available");
                }
            } else {
                LOG_ERROR("Failed to initialize camera manager");
            }
        }
    }
}

void CameraPanel::RenderDeviceControls() {
    // 第一行：设备选择
    RenderDeviceSelection();
    
    ImGui::Separator();
    
    // 第二行：主要控制按钮
    ImGui::Text("主要操作:");
    ImGui::SameLine();
    
    // 大号拍照按钮
    if (ImGui::Button("[拍照]", ImVec2(80, 40))) {
        LOG_INFO("Capture button clicked");
        CaptureImage();
    }
    ImGui::SameLine();
    
    // 录像按钮
    if (ImGui::Button("[录像]", ImVec2(80, 40))) {
        LOG_INFO("Record button clicked");
        ToggleRecording();
    }
    ImGui::SameLine();
    
    // 预览控制
    if (m_previewActive) {
        if (ImGui::Button("[停止预览]", ImVec2(100, 40))) {
            LOG_INFO("Stop preview button clicked");
            StopPreview();
        }
    } else {
        if (ImGui::Button("[开始预览]", ImVec2(100, 40))) {
            LOG_INFO("Start preview button clicked");
            StartPreview();
        }
    }
    
    ImGui::SameLine();
    
    // 分辨率选择 - 显示设备支持的实际分辨率
    ImGui::Text("分辨率:");
    ImGui::SameLine();
    RenderResolutionSelector();
}

void CameraPanel::RenderPreviewArea() {
    // 计算预览区域尺寸（保持比例）
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImVec2 previewSize = CalculatePreviewSize(availableSize, m_currentResolution);
    
    // 居中显示预览
    ImVec2 centerPos = ImVec2(
        (availableSize.x - previewSize.x) * 0.5f,
        (availableSize.y - previewSize.y) * 0.5f
    );
    ImGui::SetCursorPos(centerPos);
    
    // 显示预览图像
    if (m_previewActive && m_connected) {
        // 获取最新的图像数据
        if (m_cameraManager) {
            Plugins::PixelFormat format;
            int width, height;
            if (m_cameraManager->CaptureImage(m_imageData, format, width, height)) {
                m_imageWidth = width;
                m_imageHeight = height;
                m_hasImageData = true;
                LOG_INFO("Image captured: " + std::to_string(width) + "x" + std::to_string(height) + ", data size: " + std::to_string(m_imageData.size()));
            } else {
                LOG_ERROR("Failed to capture image");
            }
        }
        
        if (m_hasImageData && !m_imageData.empty()) {
            // 使用真实的图像数据绘制预览
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 previewStart = ImVec2(windowPos.x + centerPos.x, windowPos.y + centerPos.y);
            ImVec2 previewEnd = ImVec2(previewStart.x + previewSize.x, previewStart.y + previewSize.y);
            
            // 绘制图像数据
            float scaleX = previewSize.x / m_imageWidth;
            float scaleY = previewSize.y / m_imageHeight;
            float scale = std::min(scaleX, scaleY);
            
            int scaledWidth = (int)(m_imageWidth * scale);
            int scaledHeight = (int)(m_imageHeight * scale);
            
            // 居中绘制
            ImVec2 imageStart = ImVec2(
                previewStart.x + (previewSize.x - scaledWidth) * 0.5f,
                previewStart.y + (previewSize.y - scaledHeight) * 0.5f
            );
            ImVec2 imageEnd = ImVec2(imageStart.x + scaledWidth, imageStart.y + scaledHeight);
            
            // 绘制图像像素（简化版本，使用更大的像素块以提高性能）
            int pixelSize = 2; // 每个像素块的大小
            for (int y = 0; y < scaledHeight; y += pixelSize) {
                for (int x = 0; x < scaledWidth; x += pixelSize) {
                    // 计算原始图像坐标
                    int srcX = (int)(x / scale);
                    int srcY = (int)(y / scale);
                    
                    if (srcX < m_imageWidth && srcY < m_imageHeight) {
                        int srcIndex = (srcY * m_imageWidth + srcX) * 3;
                        if (srcIndex + 2 < m_imageData.size()) {
                            uint8_t r = m_imageData[srcIndex];
                            uint8_t g = m_imageData[srcIndex + 1];
                            uint8_t b = m_imageData[srcIndex + 2];
                            
                            ImVec2 pixelPos = ImVec2(imageStart.x + x, imageStart.y + y);
                            ImVec2 pixelEnd = ImVec2(pixelPos.x + pixelSize, pixelPos.y + pixelSize);
                            drawList->AddRectFilled(pixelPos, pixelEnd, IM_COL32(r, g, b, 255));
                        }
                    }
                }
            }
            
            // 绘制边框
            drawList->AddRect(imageStart, imageEnd, IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);
            
            // 绘制信息覆盖层
            ImVec2 infoStart = ImVec2(imageStart.x, imageStart.y);
            ImVec2 infoEnd = ImVec2(imageEnd.x, imageStart.y + 30);
            drawList->AddRectFilled(infoStart, infoEnd, IM_COL32(0, 0, 0, 128));
            
            // 绘制分辨率信息
            std::string resText = std::to_string(m_imageWidth) + "x" + std::to_string(m_imageHeight);
            ImVec2 textPos = ImVec2(imageStart.x + 10, imageStart.y + 8);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), resText.c_str());
            
            // 绘制时间戳
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
            ImVec2 timePos = ImVec2(imageEnd.x - 80, imageStart.y + 8);
            drawList->AddText(timePos, IM_COL32(255, 255, 255, 255), ss.str().c_str());
        } else {
            // 没有图像数据时显示占位符
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 previewStart = ImVec2(windowPos.x + centerPos.x, windowPos.y + centerPos.y);
            ImVec2 previewEnd = ImVec2(previewStart.x + previewSize.x, previewStart.y + previewSize.y);
            
            drawList->AddRectFilled(previewStart, previewEnd, IM_COL32(20, 20, 20, 255));
            drawList->AddRect(previewStart, previewEnd, IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);
            
            ImVec2 center = ImVec2((previewStart.x + previewEnd.x) * 0.5f, (previewStart.y + previewEnd.y) * 0.5f);
            ImVec2 textPos = ImVec2(center.x - 60, center.y - 10);
            drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), "正在获取图像...");
        }
        
        // 创建一个透明的ImGui区域用于交互
        ImGui::Dummy(previewSize);
    } else {
        // 占位符
        ImGui::Dummy(previewSize);
        ImGui::SetCursorPos(ImVec2(centerPos.x + previewSize.x * 0.5f - 50, centerPos.y + previewSize.y * 0.5f));
        ImGui::Text("预览区域");
        if (!m_previewActive) {
            ImGui::SetCursorPos(ImVec2(centerPos.x + previewSize.x * 0.5f - 80, centerPos.y + previewSize.y * 0.5f + 20));
            ImGui::Text("点击开始预览");
        } else if (!m_connected) {
            ImGui::SetCursorPos(ImVec2(centerPos.x + previewSize.x * 0.5f - 80, centerPos.y + previewSize.y * 0.5f + 20));
            ImGui::Text("请先连接设备");
        }
    }
    
    // 双击全屏
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        ToggleFullscreen();
    }
    
    // 显示实时信息覆盖层
    RenderPreviewOverlay();
    
    // 显示网格线
    if (m_showGridLines) {
        RenderGridLines(centerPos, previewSize);
    }
    
    // 在预览区域下方显示简化的控制选项
    ImGui::SetCursorPos(ImVec2(0, centerPos.y + previewSize.y + 10));
    
    // 画质调节（简化版）
    ImGui::Text("画质调节:");
    ImGui::SameLine();
    if (ImGui::SliderInt("亮度", &m_brightness, 0, 100)) {
        if (m_cameraManager) {
            m_cameraManager->SetBrightness(m_brightness);
        }
    }
    ImGui::SameLine();
    if (ImGui::SliderInt("对比度", &m_contrast, 0, 100)) {
        if (m_cameraManager) {
            m_cameraManager->SetContrast(m_contrast);
        }
    }
    
    // 拍摄模式（简化版）
    ImGui::Text("拍摄模式:");
    ImGui::SameLine();
    const char* modes[] = { "普通", "文档", "身份证", "A4" };
    int mode = static_cast<int>(m_captureMode);
    if (ImGui::Combo("##mode", &mode, modes, IM_ARRAYSIZE(modes))) {
        m_captureMode = static_cast<CaptureMode>(mode);
    }
    ImGui::SameLine();
    ImGui::Checkbox("网格线", &m_showGridLines);
}

void CameraPanel::RenderControlPanel() {
    // 主要操作按钮组
    ImGui::Text("主要操作");
    ImGui::Separator();
    
    // 大号拍照按钮（类似真实相机）
    ImVec2 buttonSize = ImVec2(80, 80);
    if (ImGui::Button("📷", buttonSize)) {
        CaptureImage();
    }
    ImGui::SameLine();
    if (ImGui::Button("🎥", buttonSize)) {
        ToggleRecording();
    }
    
    // 设备控制
    ImGui::Spacing();
    ImGui::Text("设备控制");
    ImGui::Separator();
    
    if (ImGui::Button("开始预览", ImVec2(120, 30))) {
        StartPreview();
    }
    ImGui::SameLine();
    if (ImGui::Button("停止预览", ImVec2(120, 30))) {
        StopPreview();
    }
    
    // 分辨率选择 - 显示设备支持的实际分辨率
    ImGui::Spacing();
    ImGui::Text("分辨率设置");
    ImGui::Separator();
    RenderResolutionSelector();
    
    // 画质调节
    ImGui::Spacing();
    ImGui::Text("画质调节");
    ImGui::Separator();
    
    if (ImGui::SliderInt("亮度", &m_brightness, 0, 100)) {
        if (m_cameraManager) {
            m_cameraManager->SetBrightness(m_brightness);
        }
    }
    if (ImGui::SliderInt("对比度", &m_contrast, 0, 100)) {
        if (m_cameraManager) {
            m_cameraManager->SetContrast(m_contrast);
        }
    }
    if (ImGui::SliderInt("饱和度", &m_saturation, 0, 100)) {
        // TODO: 实现饱和度设置
    }
    
    // 重置按钮
    if (ImGui::Button("重置设置", ImVec2(120, 25))) {
        m_brightness = m_contrast = m_saturation = 50;
        if (m_cameraManager) {
            m_cameraManager->SetBrightness(m_brightness);
            m_cameraManager->SetContrast(m_contrast);
        }
    }
    
    // 拍摄模式
    ImGui::Spacing();
    RenderDocumentMode();
    
    // 图像增强
    ImGui::Spacing();
    RenderImageEnhancement();
}

void CameraPanel::RenderDocumentMode() {
    ImGui::Text("拍摄模式");
    ImGui::Separator();
    
    const char* modes[] = { "普通模式", "文档模式", "身份证模式", "A4文档模式" };
    int mode = static_cast<int>(m_captureMode);
    for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
        if (ImGui::RadioButton(modes[i], mode == i)) {
            mode = i;
            m_captureMode = static_cast<CaptureMode>(mode);
        }
    }
    
    if (m_captureMode == CaptureMode::Document) {
        ImGui::Text("文档检测:");
        ImGui::SameLine();
        if (ImGui::Button("自动检测边缘")) {
            DetectDocumentEdges();
        }
        
        ImGui::Text("边缘调整:");
        ImGui::SliderFloat("阈值", &m_edgeThreshold, 0.1f, 1.0f);
    }
    
    // 网格线选项
    ImGui::Checkbox("显示网格线", &m_showGridLines);
}

void CameraPanel::RenderImageEnhancement() {
    ImGui::Text("图像增强");
    ImGui::Separator();
    
    ImGui::Checkbox("自动增强", &m_autoEnhance);
    
    if (!m_autoEnhance) {
        ImGui::SliderInt("锐化", &m_sharpness, 0, 100);
        ImGui::SliderInt("降噪", &m_denoise, 0, 100);
        ImGui::Checkbox("自动旋转", &m_autoRotate);
    }
}

void CameraPanel::RenderPreviewOverlay() {
    // 在预览区域右上角显示信息
    ImVec2 overlayPos = ImVec2(ImGui::GetWindowWidth() - 200, 10);
    ImGui::SetCursorPos(overlayPos);
    
    ImGui::BeginChild("InfoOverlay", ImVec2(190, 80), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("分辨率: %dx%d", m_currentResolution.width, m_currentResolution.height);
    ImGui::Text("帧率: %d fps", m_currentFPS);
    ImGui::Text("格式: %s", m_currentFormat.c_str());
    ImGui::EndChild();
}

void CameraPanel::RenderImageInfo() {
    ImGui::Text("图像信息");
    ImGui::Separator();
    
    ImGui::Text("分辨率: %dx%d", m_currentResolution.width, m_currentResolution.height);
    ImGui::SameLine();
    ImGui::Text("帧率: %d fps", m_currentFPS);
    ImGui::SameLine();
    ImGui::Text("格式: %s", m_currentFormat.c_str());
    ImGui::SameLine();
    ImGui::Text("状态: %s", m_previewActive ? "预览中" : "已停止");
}

void CameraPanel::UpdateLayout() {
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // 根据窗口大小调整布局
    if (windowSize.x < 800) {
        m_layoutMode = LayoutMode::Compact;
    } else if (windowSize.x < 1200) {
        m_layoutMode = LayoutMode::Standard;
    } else {
        m_layoutMode = LayoutMode::Wide;
    }
}

ImVec2 CameraPanel::CalculatePreviewSize(const ImVec2& availableSize, const Plugins::Resolution& resolution) {
    float aspectRatio = static_cast<float>(resolution.width) / resolution.height;
    
    float width = availableSize.x;
    float height = width / aspectRatio;
    
    if (height > availableSize.y) {
        height = availableSize.y;
        width = height * aspectRatio;
    }
    
    return ImVec2(width, height);
}

void CameraPanel::CaptureImage() {
    if (!m_cameraManager || !m_connected) {
        LOG_WARNING("Camera not connected, cannot capture image");
        return;
    }
    
    std::vector<uint8_t> imageData;
    Plugins::PixelFormat format;
    int width, height;
    if (m_cameraManager->CaptureImage(imageData, format, width, height)) {
        LOG_INFO("Image captured successfully");
        // TODO: 保存图像或显示在界面上
    } else {
        LOG_ERROR("Failed to capture image");
    }
}

void CameraPanel::ToggleRecording() {
    m_recording = !m_recording;
    LOG_INFO(m_recording ? "Recording started" : "Recording stopped");
    // TODO: 实现录像功能
}

void CameraPanel::ToggleFullscreen() {
    m_fullscreen = !m_fullscreen;
    LOG_INFO(m_fullscreen ? "Entered fullscreen mode" : "Exited fullscreen mode");
    // TODO: 实现全屏功能
}

void CameraPanel::ExitFullscreen() {
    if (m_fullscreen) {
        m_fullscreen = false;
        LOG_INFO("Exited fullscreen mode");
    }
}

void CameraPanel::RefreshDeviceList() {
    LOG_INFO("Refreshing device list...");
    if (m_cameraManager) {
        m_availableCameras = m_cameraManager->GetAvailableCameras();
        LOG_INFO("Device list refreshed, found " + std::to_string(m_availableCameras.size()) + " devices");
        
        // 显示设备信息
        for (size_t i = 0; i < m_availableCameras.size(); ++i) {
            LOG_INFO("Device " + std::to_string(i) + ": " + m_availableCameras[i].name + 
                    " (" + m_availableCameras[i].devicePath + ")");
        }
    } else {
        LOG_ERROR("Camera manager not available for device refresh");
    }
}

void CameraPanel::DetectDocumentEdges() {
    LOG_INFO("Detecting document edges with threshold: " + std::to_string(m_edgeThreshold));
    // TODO: 实现文档边缘检测
}

void CameraPanel::StartPreview() {
    if (!m_cameraManager || !m_connected) {
        LOG_WARNING("Camera not connected, cannot start preview");
        return;
    }
    
    if (m_cameraManager->StartPreview()) {
        m_previewActive = true;
        LOG_INFO("Preview started");
    } else {
        LOG_ERROR("Failed to start preview");
    }
}

void CameraPanel::StopPreview() {
    if (m_cameraManager && m_previewActive) {
        m_cameraManager->StopPreview();
        m_previewActive = false;
        LOG_INFO("Preview stopped");
    }
}

void CameraPanel::RenderResolutionSelector() {
    // 获取当前设备支持的分辨率
    static int selectedResolution = 0;
    static std::vector<std::string> resolutionStrings;
    static std::vector<Plugins::Resolution> availableResolutions;
    
    // 如果设备已连接，获取支持的分辨率
    if (m_connected && m_cameraManager && !m_availableCameras.empty() && m_selectedDevice < m_availableCameras.size()) {
        availableResolutions = m_availableCameras[m_selectedDevice].supportedResolutions;
        if (availableResolutions.empty()) {
            // 如果没有获取到设备实际支持的分辨率，使用常见分辨率作为备选
            availableResolutions = {
                Plugins::Resolution(320, 240, 30, "QVGA"),
                Plugins::Resolution(640, 480, 30, "VGA"),
                Plugins::Resolution(800, 600, 30, "SVGA"),
                Plugins::Resolution(1024, 768, 30, "XGA"),
                Plugins::Resolution(1280, 720, 30, "HD"),
                Plugins::Resolution(1280, 960, 30, "SXGA"),
                Plugins::Resolution(1600, 1200, 30, "UXGA"),
                Plugins::Resolution(1920, 1080, 30, "FHD")
            };
        }
        
        // 更新分辨率字符串列表
        resolutionStrings.clear();
        for (const auto& res : availableResolutions) {
            resolutionStrings.push_back(res.description + " (" + std::to_string(res.width) + "x" + std::to_string(res.height) + "@" + std::to_string(res.fps) + ")");
        }
        
        // 找到当前分辨率在列表中的索引
        selectedResolution = 0;
        for (size_t i = 0; i < availableResolutions.size(); i++) {
            if (availableResolutions[i].width == m_currentResolution.width && 
                availableResolutions[i].height == m_currentResolution.height) {
                selectedResolution = static_cast<int>(i);
                break;
            }
        }
    } else {
        // 设备未连接时显示常见分辨率
        availableResolutions = {
            Plugins::Resolution(320, 240, 30, "QVGA"),
            Plugins::Resolution(640, 480, 30, "VGA"),
            Plugins::Resolution(800, 600, 30, "SVGA"),
            Plugins::Resolution(1024, 768, 30, "XGA"),
            Plugins::Resolution(1280, 720, 30, "HD"),
            Plugins::Resolution(1280, 960, 30, "SXGA"),
            Plugins::Resolution(1600, 1200, 30, "UXGA"),
            Plugins::Resolution(1920, 1080, 30, "FHD")
        };
        resolutionStrings = {
            "QVGA (320x240@30)",
            "VGA (640x480@30)",
            "SVGA (800x600@30)",
            "XGA (1024x768@30)",
            "HD (1280x720@30)",
            "SXGA (1280x960@30)",
            "UXGA (1600x1200@30)",
            "FHD (1920x1080@30)"
        };
        selectedResolution = 1; // 默认选择VGA
    }
    
    // 创建分辨率选项数组
    std::vector<const char*> resolutionOptions;
    for (const auto& str : resolutionStrings) {
        resolutionOptions.push_back(str.c_str());
    }
    
    if (!resolutionOptions.empty() && ImGui::Combo("##resolution", &selectedResolution, resolutionOptions.data(), resolutionOptions.size())) {
        // 更新分辨率
        if (selectedResolution < static_cast<int>(availableResolutions.size())) {
            Plugins::Resolution newResolution = availableResolutions[selectedResolution];
            
            // 检查分辨率是否真的改变了
            if (newResolution.width != m_currentResolution.width || 
                newResolution.height != m_currentResolution.height ||
                newResolution.fps != m_currentResolution.fps) {
                
                m_currentResolution = newResolution;
                
                // 如果摄像头已连接，应用新的分辨率
                if (m_cameraManager && m_connected) {
                    // 如果正在预览，需要重新启动预览以应用新分辨率
                    bool wasPreviewing = m_previewActive;
                    if (wasPreviewing) {
                        StopPreview();
                    }
                    
                    // 设置新分辨率
                    if (m_cameraManager->SetResolution(m_currentResolution.width, m_currentResolution.height, m_currentResolution.fps)) {
                        LOG_INFO("Resolution set to: " + std::to_string(m_currentResolution.width) + "x" + std::to_string(m_currentResolution.height) + "@" + std::to_string(m_currentResolution.fps));
                        
                        // 如果之前正在预览，重新启动预览
                        if (wasPreviewing) {
                            StartPreview();
                        }
                    } else {
                        LOG_ERROR("Failed to set resolution: " + m_cameraManager->GetLastError());
                    }
                }
            }
        }
    }
}

void CameraPanel::RenderGridLines(const ImVec2& startPos, const ImVec2& size) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 gridStart = ImVec2(windowPos.x + startPos.x, windowPos.y + startPos.y);
    
    ImU32 gridColor = IM_COL32(255, 255, 255, 100);
    
    // 垂直网格线
    for (int i = 1; i < 3; i++) {
        float x = gridStart.x + (size.x * i / 3);
        drawList->AddLine(ImVec2(x, gridStart.y), ImVec2(x, gridStart.y + size.y), gridColor);
    }
    
    // 水平网格线
    for (int i = 1; i < 3; i++) {
        float y = gridStart.y + (size.y * i / 3);
        drawList->AddLine(ImVec2(gridStart.x, y), ImVec2(gridStart.x + size.x, y), gridColor);
    }
}

} // namespace AsTestTool
