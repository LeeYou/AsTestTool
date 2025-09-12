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
    // 初始化默认分辨率 - 使用更通用的分辨率
    m_currentResolution = Plugins::Resolution(640, 480);
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
    
    // 适应新的面板尺寸 - 根据布局模式调整
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    // 根据布局模式和可用空间动态调整各区域高度
    float controlHeight, infoHeight;
    float minControlHeight = 120.0f; // 最小控制区域高度，确保所有按钮都能显示
    
    switch (m_layoutMode) {
        case LayoutMode::Compact:
            controlHeight = std::max(minControlHeight, availableSize.y * 0.25f); // 至少25%高度
            infoHeight = std::max(100.0f, availableSize.y * 0.25f); // 确保有足够空间显示所有控件
            break;
        case LayoutMode::Standard:
            controlHeight = std::max(minControlHeight, availableSize.y * 0.3f); // 至少30%高度
            infoHeight = std::max(120.0f, availableSize.y * 0.3f); // 确保有足够空间显示所有控件
            break;
        case LayoutMode::Wide:
            controlHeight = std::max(minControlHeight, availableSize.y * 0.35f); // 至少35%高度
            infoHeight = std::max(140.0f, availableSize.y * 0.35f); // 确保有足够空间显示所有控件
            break;
    }
    
    // 设备选择和控制区域 - 顶部
    ImGui::BeginChild("DeviceControls", ImVec2(-1, controlHeight), true);
    RenderDeviceControls();
    ImGui::EndChild();
    
    // 预览区域 - 中间主要区域
    // 确保预览区域不会占用过多空间，为ImageInfo区域留出足够空间
    float previewHeight = availableSize.y - controlHeight - infoHeight;
    previewHeight = std::max(100.0f, previewHeight); // 最小预览高度
    previewHeight = std::min(previewHeight, availableSize.y * 0.6f); // 最大不超过60%高度
    
    ImGui::BeginChild("PreviewArea", ImVec2(-1, previewHeight), true, ImGuiWindowFlags_NoScrollbar);
    RenderPreviewArea();
    ImGui::EndChild();
    
    // 图像信息 - 底部
    ImGui::BeginChild("ImageInfo", ImVec2(-1, infoHeight), true);
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
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    float availableWidth = availableSize.x;
    
    // 根据可用宽度决定布局策略
    bool useCompactLayout = (availableWidth < 400);
    
    if (useCompactLayout) {
        // 紧凑布局：垂直排列
        ImGui::Text("设备选择:");
        
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
        
        // 连接状态指示
        ImVec4 statusColor = m_connected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImGui::TextColored(statusColor, m_connected ? "● 已连接" : "● 未连接");
        
        // 操作按钮 - 水平排列但较小
        float buttonWidth = std::min(70.0f, (availableWidth - 20) / 2.0f);
        buttonWidth = std::max(60.0f, buttonWidth);
        
        if (ImGui::Button("刷新设备", ImVec2(buttonWidth, 25))) {
            RefreshDeviceList();
        }
        
        ImGui::SameLine();
        
        if (m_connected) {
            if (ImGui::Button("断开连接", ImVec2(buttonWidth, 25))) {
                LOG_INFO("Disconnecting camera...");
                StopPreview();
                if (m_cameraManager) {
                    m_cameraManager->CloseCamera();
                }
                m_connected = false;
                LOG_INFO("Camera disconnected");
            }
        } else {
            if (ImGui::Button("连接设备", ImVec2(buttonWidth, 25))) {
                LOG_INFO("Connecting to camera...");
                if (m_cameraManager && m_cameraManager->Initialize()) {
                    auto cameras = m_cameraManager->GetAvailableCameras();
                    if (!cameras.empty() && m_selectedDevice < cameras.size()) {
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
    } else {
        // 标准布局：水平排列
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
                    auto cameras = m_cameraManager->GetAvailableCameras();
                    if (!cameras.empty() && m_selectedDevice < cameras.size()) {
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
}

void CameraPanel::RenderDeviceControls() {
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    float availableWidth = availableSize.x;
    float availableHeight = availableSize.y;
    
    // 根据可用空间决定布局策略
    bool useVerticalLayout = (availableWidth < 400 || availableHeight < 100);
    bool useCompactButtons = (availableWidth < 500);
    
    // 第一行：设备选择
    RenderDeviceSelection();
    
    ImGui::Separator();
    
    // 主要操作按钮区域
    ImGui::Text("主要操作:");
    
    if (useVerticalLayout) {
        // 垂直布局：按钮垂直排列
        float buttonWidth = std::min(120.0f, availableWidth * 0.8f);
        float buttonHeight = 30.0f;
        
        // 第一行按钮
        if (ImGui::Button("📷 拍照", ImVec2(buttonWidth, buttonHeight))) {
            LOG_INFO("Capture button clicked");
            CaptureImage();
        }
        
        if (ImGui::Button("🎥 录像", ImVec2(buttonWidth, buttonHeight))) {
            LOG_INFO("Record button clicked");
            ToggleRecording();
        }
        
        // 预览控制按钮
        if (m_previewActive) {
            if (ImGui::Button("⏹ 停止预览", ImVec2(buttonWidth, buttonHeight))) {
                LOG_INFO("Stop preview button clicked");
                StopPreview();
            }
        } else {
            if (ImGui::Button("▶ 开始预览", ImVec2(buttonWidth, buttonHeight))) {
                LOG_INFO("Start preview button clicked");
                StartPreview();
            }
        }
    } else {
        // 水平布局：按钮水平排列
        float buttonWidth;
        if (useCompactButtons) {
            // 紧凑模式：较小的按钮
            buttonWidth = std::min(70.0f, (availableWidth - 50) / 3.0f);
        } else {
            // 标准模式：较大的按钮
            buttonWidth = std::min(100.0f, (availableWidth - 100) / 3.0f);
        }
        buttonWidth = std::max(60.0f, buttonWidth); // 最小宽度
        float buttonHeight = useCompactButtons ? 30.0f : 35.0f;
        
        // 拍照按钮
        if (ImGui::Button("📷 拍照", ImVec2(buttonWidth, buttonHeight))) {
            LOG_INFO("Capture button clicked");
            CaptureImage();
        }
        ImGui::SameLine();
        
        // 录像按钮
        if (ImGui::Button("🎥 录像", ImVec2(buttonWidth, buttonHeight))) {
            LOG_INFO("Record button clicked");
            ToggleRecording();
        }
        ImGui::SameLine();
        
        // 预览控制按钮
        if (m_previewActive) {
            if (ImGui::Button("⏹ 停止预览", ImVec2(buttonWidth + 10, buttonHeight))) {
                LOG_INFO("Stop preview button clicked");
                StopPreview();
            }
        } else {
            if (ImGui::Button("▶ 开始预览", ImVec2(buttonWidth + 10, buttonHeight))) {
                LOG_INFO("Start preview button clicked");
                StartPreview();
            }
        }
    }
    
    // 分辨率选择区域
    if (availableHeight > 60) { // 只有在有足够高度时才显示分辨率选择
        ImGui::NewLine();
        ImGui::Text("分辨率:");
        ImGui::SameLine();
        RenderResolutionSelector();
    }
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
                m_currentFormat = GetPixelFormatString(format);
                // 减少日志输出频率，避免刷屏
                static int logCounter = 0;
                if (++logCounter % 30 == 0) { // 每30帧输出一次日志
                    LOG_INFO("Preview frame: " + std::to_string(width) + "x" + std::to_string(height) + ", data size: " + std::to_string(m_imageData.size()));
                }
            } else {
                // 减少错误日志频率
                static int errorCounter = 0;
                if (++errorCounter % 60 == 0) { // 每60帧输出一次错误日志
                    LOG_ERROR("Failed to capture preview frame");
                }
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
            
            // 移除信息覆盖层，避免与ImageInfo区域重复显示
        } else {
            // 没有图像数据时显示占位符
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 previewStart = ImVec2(windowPos.x + centerPos.x, windowPos.y + centerPos.y);
            ImVec2 previewEnd = ImVec2(previewStart.x + previewSize.x, previewStart.y + previewSize.y);
            
            // 绘制渐变背景
            drawList->AddRectFilled(previewStart, previewEnd, IM_COL32(30, 30, 30, 255));
            drawList->AddRect(previewStart, previewEnd, IM_COL32(80, 80, 80, 255), 0.0f, 0, 2.0f);
            
            // 绘制摄像头图标和状态文本
            ImVec2 center = ImVec2((previewStart.x + previewEnd.x) * 0.5f, (previewStart.y + previewEnd.y) * 0.5f);
            
            // 摄像头图标（更大更清晰）
            ImVec2 iconPos = ImVec2(center.x - 30, center.y - 50);
            drawList->AddText(iconPos, IM_COL32(120, 120, 120, 255), "📷");
            
            // 状态文本（更清晰的层次）
            ImVec2 textPos = ImVec2(center.x - 60, center.y + 5);
            drawList->AddText(textPos, IM_COL32(180, 180, 180, 255), "正在获取图像...");
            
            // 提示文本（更友好的提示）
            ImVec2 hintPos = ImVec2(center.x - 90, center.y + 25);
            drawList->AddText(hintPos, IM_COL32(120, 120, 120, 255), "请稍候，正在连接摄像头");
            
            // 添加加载指示器
            ImVec2 loadingPos = ImVec2(center.x - 10, center.y + 45);
            drawList->AddText(loadingPos, IM_COL32(100, 150, 200, 255), "● 连接中");
        }
        
        // 创建一个透明的ImGui区域用于交互
        ImGui::Dummy(previewSize);
    } else {
        // 非预览状态下的占位符
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 previewStart = ImVec2(windowPos.x + centerPos.x, windowPos.y + centerPos.y);
        ImVec2 previewEnd = ImVec2(previewStart.x + previewSize.x, previewStart.y + previewSize.y);
        
        // 绘制背景
        drawList->AddRectFilled(previewStart, previewEnd, IM_COL32(40, 40, 40, 255));
        drawList->AddRect(previewStart, previewEnd, IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);
        
        // 绘制中心内容
        ImVec2 center = ImVec2((previewStart.x + previewEnd.x) * 0.5f, (previewStart.y + previewEnd.y) * 0.5f);
        
        // 摄像头图标（更大更清晰）
        ImVec2 iconPos = ImVec2(center.x - 30, center.y - 50);
        drawList->AddText(iconPos, IM_COL32(80, 80, 80, 255), "📷");
        
        // 状态文本（更清晰的层次）
        if (!m_previewActive) {
            ImVec2 textPos = ImVec2(center.x - 50, center.y + 5);
            drawList->AddText(textPos, IM_COL32(160, 160, 160, 255), "摄像头预览");
            
            ImVec2 hintPos = ImVec2(center.x - 80, center.y + 25);
            drawList->AddText(hintPos, IM_COL32(120, 120, 120, 255), "点击 [开始预览] 按钮开始");
            
            // 添加连接状态指示器
            ImVec2 statusPos = ImVec2(center.x - 20, center.y + 45);
            if (m_connected) {
                drawList->AddText(statusPos, IM_COL32(100, 200, 100, 255), "● 已连接");
            } else {
                drawList->AddText(statusPos, IM_COL32(200, 100, 100, 255), "● 未连接");
            }
        } else if (!m_connected) {
            ImVec2 textPos = ImVec2(center.x - 60, center.y + 5);
            drawList->AddText(textPos, IM_COL32(200, 100, 100, 255), "设备未连接");
            
            ImVec2 hintPos = ImVec2(center.x - 100, center.y + 25);
            drawList->AddText(hintPos, IM_COL32(120, 120, 120, 255), "请先选择并连接摄像头设备");
        }
        
        // 创建一个透明的ImGui区域用于交互
        ImGui::Dummy(previewSize);
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
    
    // 移除这些控件，它们将被移到ImageInfo区域
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
    // 移除预览区域的信息覆盖层，避免与占位背景图重叠
}

void CameraPanel::RenderImageInfo() {
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    float availableHeight = availableSize.y;
    
    // 根据可用高度决定布局策略
    bool useCompactLayout = (availableHeight < 100);
    bool useVerticalLayout = (availableSize.x < 400);
    
    ImGui::Text("图像信息");
    ImGui::Separator();
    
    // 显示实际检测到的分辨率，而不是硬编码值
    if (m_connected && m_hasImageData && m_imageWidth > 0 && m_imageHeight > 0) {
        ImGui::Text("分辨率: %dx%d", m_imageWidth, m_imageHeight);
    } else if (m_connected) {
        ImGui::Text("分辨率: %dx%d", m_currentResolution.width, m_currentResolution.height);
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "分辨率: 未检测");
    }
    
    ImGui::SameLine();
    ImGui::Text("帧率: %d fps", m_currentFPS);
    ImGui::SameLine();
    ImGui::Text("格式: %s", m_currentFormat.c_str());
    ImGui::SameLine();
    ImGui::Text("状态: %s", m_previewActive ? "预览中" : "已停止");
    
    // 只有在有足够空间时才显示额外的控件
    if (availableHeight > 60) {
        ImGui::Separator();
        
        if (useCompactLayout) {
            // 紧凑布局：垂直排列
            ImGui::Text("画质调节:");
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
            
            ImGui::Text("拍摄模式:");
            const char* modes[] = { "普通", "文档", "身份证", "A4" };
            int mode = static_cast<int>(m_captureMode);
            if (ImGui::Combo("##mode", &mode, modes, IM_ARRAYSIZE(modes))) {
                m_captureMode = static_cast<CaptureMode>(mode);
            }
            ImGui::SameLine();
            ImGui::Checkbox("网格线", &m_showGridLines);
        } else {
            // 标准布局：水平排列
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
    }
}

void CameraPanel::UpdateLayout() {
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // 根据窗口大小调整布局 - 更精细的断点控制
    if (windowSize.x < 400) {
        m_layoutMode = LayoutMode::Compact;  // 超紧凑模式
    } else if (windowSize.x < 600) {
        m_layoutMode = LayoutMode::Compact;  // 紧凑模式
    } else if (windowSize.x < 1000) {
        m_layoutMode = LayoutMode::Standard; // 标准模式
    } else {
        m_layoutMode = LayoutMode::Wide;     // 宽屏模式
    }
    
    // 根据高度调整垂直布局
    if (windowSize.y < 300) {
        // 高度不足时，减少控制区域高度
        m_layoutMode = LayoutMode::Compact;
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

std::string CameraPanel::GetPixelFormatString(Plugins::PixelFormat format) const {
    switch (format) {
        case Plugins::PixelFormat::YUV420:
            return "YUV420";
        case Plugins::PixelFormat::YUV422:
            return "YUV422";
        case Plugins::PixelFormat::RGB24:
            return "RGB24";
        case Plugins::PixelFormat::RGB32:
            return "RGB32";
        case Plugins::PixelFormat::MJPG:
            return "MJPG";
        case Plugins::PixelFormat::H264:
            return "H264";
        case Plugins::PixelFormat::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace AsTestTool
