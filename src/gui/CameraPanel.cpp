#include "gui/CameraPanel.h"
#include "gui/PanelUiHelpers.h"
#include "plugins/CameraManager.h"
#include "utils/Logger.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <shellapi.h>
#endif

namespace AsTestTool {

namespace {

std::string BuildCameraRecordingTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo{};
#ifdef _WIN32
    localtime_s(&timeInfo, &currentTime);
#else
    localtime_r(&currentTime, &timeInfo);
#endif
    std::ostringstream stream;
    stream << std::put_time(&timeInfo, "%Y%m%d_%H%M%S");
    return stream.str();
}

std::string QuoteCommandArgument(const std::string& value) {
    return std::string("\"") + value + "\"";
}

bool DirectoryContainsRecordingFrames(const std::string& directory) {
    if (directory.empty()) {
        return false;
    }

    std::error_code errorCode;
    const std::filesystem::path directoryPath(directory);
    if (!std::filesystem::exists(directoryPath, errorCode) || errorCode) {
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath, errorCode)) {
        if (errorCode) {
            return false;
        }

        if (!entry.is_regular_file(errorCode) || errorCode) {
            continue;
        }

        const std::string fileName = entry.path().filename().string();
        if (fileName.rfind("frame_", 0) == 0 && entry.path().extension() == ".bmp") {
            return true;
        }
    }

    return false;
}

bool WriteBmpFrame(const std::string& path,
                   const std::vector<uint8_t>& imageData,
                   int width,
                   int height,
                   Plugins::PixelFormat format) {
    if (imageData.empty() || width <= 0 || height <= 0) {
        return false;
    }

    int sourceChannels = 0;
    switch (format) {
        case Plugins::PixelFormat::RGB24:
        case Plugins::PixelFormat::BGR24:
            sourceChannels = 3;
            break;
        case Plugins::PixelFormat::RGB32:
            sourceChannels = 4;
            break;
        default:
            return false;
    }

    const std::size_t requiredSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(sourceChannels);
    if (imageData.size() < requiredSize) {
        return false;
    }

    const int rowStride = ((width * 3) + 3) & ~3;
    const int imageSize = rowStride * height;
    const int fileSize = 54 + imageSize;

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    unsigned char header[54] = {};
    header[0] = 'B';
    header[1] = 'M';
    header[2] = static_cast<unsigned char>(fileSize & 0xFF);
    header[3] = static_cast<unsigned char>((fileSize >> 8) & 0xFF);
    header[4] = static_cast<unsigned char>((fileSize >> 16) & 0xFF);
    header[5] = static_cast<unsigned char>((fileSize >> 24) & 0xFF);
    header[10] = 54;
    header[14] = 40;
    header[18] = static_cast<unsigned char>(width & 0xFF);
    header[19] = static_cast<unsigned char>((width >> 8) & 0xFF);
    header[20] = static_cast<unsigned char>((width >> 16) & 0xFF);
    header[21] = static_cast<unsigned char>((width >> 24) & 0xFF);
    header[22] = static_cast<unsigned char>(height & 0xFF);
    header[23] = static_cast<unsigned char>((height >> 8) & 0xFF);
    header[24] = static_cast<unsigned char>((height >> 16) & 0xFF);
    header[25] = static_cast<unsigned char>((height >> 24) & 0xFF);
    header[26] = 1;
    header[28] = 24;
    header[34] = static_cast<unsigned char>(imageSize & 0xFF);
    header[35] = static_cast<unsigned char>((imageSize >> 8) & 0xFF);
    header[36] = static_cast<unsigned char>((imageSize >> 16) & 0xFF);
    header[37] = static_cast<unsigned char>((imageSize >> 24) & 0xFF);
    file.write(reinterpret_cast<const char*>(header), sizeof(header));

    std::vector<unsigned char> row(static_cast<std::size_t>(rowStride), 0);
    for (int outputRow = 0; outputRow < height; ++outputRow) {
        const int sourceRow = height - 1 - outputRow;
        for (int x = 0; x < width; ++x) {
            const std::size_t sourceIndex = (static_cast<std::size_t>(sourceRow) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * static_cast<std::size_t>(sourceChannels);
            const std::size_t destIndex = static_cast<std::size_t>(x) * 3;

            if (format == Plugins::PixelFormat::RGB24) {
                row[destIndex + 0] = imageData[sourceIndex + 2];
                row[destIndex + 1] = imageData[sourceIndex + 1];
                row[destIndex + 2] = imageData[sourceIndex + 0];
            } else if (format == Plugins::PixelFormat::BGR24) {
                row[destIndex + 0] = imageData[sourceIndex + 0];
                row[destIndex + 1] = imageData[sourceIndex + 1];
                row[destIndex + 2] = imageData[sourceIndex + 2];
            } else {
                row[destIndex + 0] = imageData[sourceIndex + 2];
                row[destIndex + 1] = imageData[sourceIndex + 1];
                row[destIndex + 2] = imageData[sourceIndex + 0];
            }
        }
        file.write(reinterpret_cast<const char*>(row.data()), rowStride);
    }

    return file.good();
}

bool DetectDocumentBounds(const std::vector<uint8_t>& imageData,
                          int width,
                          int height,
                          Plugins::PixelFormat format,
                          float threshold,
                          float& left,
                          float& top,
                          float& right,
                          float& bottom) {
    if (imageData.empty() || width <= 2 || height <= 2) {
        return false;
    }

    int channels = 0;
    switch (format) {
        case Plugins::PixelFormat::RGB24:
        case Plugins::PixelFormat::BGR24:
            channels = 3;
            break;
        case Plugins::PixelFormat::RGB32:
            channels = 4;
            break;
        default:
            return false;
    }

    const std::size_t requiredSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(channels);
    if (imageData.size() < requiredSize) {
        return false;
    }

    auto sampleLuminance = [&](int x, int y) -> int {
        const std::size_t index = (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * static_cast<std::size_t>(channels);
        int r = 0;
        int g = 0;
        int b = 0;
        if (format == Plugins::PixelFormat::BGR24) {
            b = imageData[index + 0];
            g = imageData[index + 1];
            r = imageData[index + 2];
        } else {
            r = imageData[index + 0];
            g = imageData[index + 1];
            b = imageData[index + 2];
        }
        return (r * 30 + g * 59 + b * 11) / 100;
    };

    const int step = std::max(1, std::min(width, height) / 240);
    const int margin = std::max(step * 2, std::min(width, height) / 25);
    const int thresholdValue = static_cast<int>(40.0f + std::clamp(threshold, 0.0f, 1.0f) * 180.0f);

    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;
    int edgeCount = 0;

    for (int y = margin; y < height - margin; y += step) {
        for (int x = margin; x < width - margin; x += step) {
            const int gx = std::abs(sampleLuminance(std::min(width - 1, x + step), y) - sampleLuminance(std::max(0, x - step), y));
            const int gy = std::abs(sampleLuminance(x, std::min(height - 1, y + step)) - sampleLuminance(x, std::max(0, y - step)));
            const int magnitude = gx + gy;
            if (magnitude >= thresholdValue) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
                ++edgeCount;
            }
        }
    }

    const int sampledWidth = std::max(1, (width - margin * 2) / step);
    const int sampledHeight = std::max(1, (height - margin * 2) / step);
    const int minEdgeCount = std::max(50, (sampledWidth * sampledHeight) / 180);

    if (edgeCount < minEdgeCount || minX >= maxX || minY >= maxY) {
        return false;
    }

    if ((maxX - minX) < width / 8 || (maxY - minY) < height / 8) {
        return false;
    }

    minX = std::max(0, minX - step * 2);
    minY = std::max(0, minY - step * 2);
    maxX = std::min(width - 1, maxX + step * 2);
    maxY = std::min(height - 1, maxY + step * 2);

    left = static_cast<float>(minX) / static_cast<float>(width);
    top = static_cast<float>(minY) / static_cast<float>(height);
    right = static_cast<float>(maxX) / static_cast<float>(width);
    bottom = static_cast<float>(maxY) / static_cast<float>(height);
    return true;
}

void RenderDocumentBoundsOverlay(const ImVec2& startPos,
                                 const ImVec2& size,
                                 bool hasDetectedDocument,
                                 float left,
                                 float top,
                                 float right,
                                 float bottom) {
    if (!hasDetectedDocument) {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 rectStart(windowPos.x + startPos.x + size.x * left, windowPos.y + startPos.y + size.y * top);
    const ImVec2 rectEnd(windowPos.x + startPos.x + size.x * right, windowPos.y + startPos.y + size.y * bottom);
    drawList->AddRect(rectStart, rectEnd, IM_COL32(255, 180, 0, 255), 0.0f, 0, 2.5f);
}

}

static bool UploadPreviewFrame(TextureRenderer* textureRenderer,
                               const std::vector<uint8_t>& imageData,
                               int width,
                               int height,
                               Plugins::PixelFormat format);

CameraPanel::CameraPanel() 
    : m_textureRenderer(std::make_unique<TextureRenderer>()) {
    LOG_INFO("CameraPanel created");
    // 初始化纹理渲染器
    m_textureRenderer->Initialize();
    // 初始化默认分辨率 - 使用更通用的分辨率
    m_currentResolution = Plugins::Resolution(640, 480);
}

CameraPanel::~CameraPanel() {
    LOG_INFO("CameraPanel destroyed");
    if (m_cameraManager) {
        StopPreview();
    }
    m_textureRenderer->Shutdown();
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

    if (m_fullscreen) {
        RenderFullscreenPreview();
    }
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
    if (ImGui::IsKeyPressed(ImGuiKey_F11) && (m_fullscreen || (m_connected && m_previewActive))) {
        ToggleFullscreen();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) && m_fullscreen) {
        ExitFullscreen();
    }
}

void CameraPanel::RenderDeviceSelection() {
    const bool hasSelectedCamera = !m_availableCameras.empty() && m_selectedDevice >= 0 && m_selectedDevice < static_cast<int>(m_availableCameras.size());
    const char* previewText = hasSelectedCamera ? m_availableCameras[m_selectedDevice].name.c_str() : "无设备";

    PanelUi::BeginInlineFieldRow("设备选择:", 0.0f, 180.0f);
    if (ImGui::BeginCombo("##device", previewText)) {
        for (int i = 0; i < static_cast<int>(m_availableCameras.size()); i++) {
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

    ImVec4 statusColor = m_connected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const std::string deviceInfo = hasSelectedCamera ? m_availableCameras[m_selectedDevice].name : std::string();
    PanelUi::RenderDeviceStatusBlock(m_connected ? "已连接" : "未连接", statusColor, deviceInfo.c_str());

    PanelUi::RenderActionButton("刷新设备", [this]() { RefreshDeviceList(); });

    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);

    PanelUi::RenderConnectionToggleButton(
        m_connected,
        [this]() {
            LOG_INFO("Connecting to camera...");
            if (m_cameraManager && m_cameraManager->Initialize()) {
                auto cameras = m_cameraManager->GetAvailableCameras();
                if (!cameras.empty() && m_selectedDevice < static_cast<int>(cameras.size())) {
                    if (m_cameraManager->OpenCamera(cameras[m_selectedDevice].id)) {
                        m_connected = true;
                        m_recording = false;
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
        },
        [this]() {
            LOG_INFO("Disconnecting camera...");
            if (!StopPreview()) {
                return;
            }
            if (m_cameraManager && !m_cameraManager->CloseCamera()) {
                LOG_ERROR("Failed to close camera: " + m_cameraManager->GetLastError());
                return;
            }
            m_connected = false;
            StopRecordingSession();
            ResetPreviewFrameState();
            LOG_INFO("Camera disconnected");
        }
    );
}

void CameraPanel::RenderDeviceControls() {
    float availableHeight = ImGui::GetContentRegionAvail().y;

    RenderDeviceSelection();

    PanelUi::RenderSectionTitle("主要操作:");
    RenderCaptureActionButtons(PanelUi::kButtonWidth, PanelUi::kButtonHeight);
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    RenderPreviewToggleButton(PanelUi::kButtonWidth, PanelUi::kButtonHeight);

    if (availableHeight > 60) {
        RenderDisplayControls(true, false, PanelUi::kButtonWidth, PanelUi::kButtonHeight, PanelUi::kButtonWidth, PanelUi::kButtonHeight);
    }
}

void CameraPanel::RenderCaptureActionButtons(float buttonWidth, float buttonHeight) {
    const bool canCapture = m_connected && m_cameraManager;
    const bool canRecord = m_connected && m_previewActive && m_cameraManager;
    PanelUi::RenderActionButton("拍照", [this]() {
        LOG_INFO("Capture button clicked");
        CaptureImage();
    }, !canCapture, buttonWidth, buttonHeight);
    PanelUi::ContinueOnSameLineIfFits(buttonWidth);
    PanelUi::RenderActionButton(m_recording ? "停止录像" : "开始录像", [this]() {
        LOG_INFO("Record button clicked");
        ToggleRecording();
    }, !canRecord, buttonWidth, buttonHeight);
}

void CameraPanel::RenderPreviewToggleButton(float buttonWidth, float buttonHeight) {
    const bool previewButtonDisabled = !m_connected && !m_previewActive;
    if (m_previewActive) {
        PanelUi::RenderActionButton("停止预览", [this]() {
            LOG_INFO("Stop preview button clicked");
            StopPreview();
        }, previewButtonDisabled, buttonWidth, buttonHeight);
    } else {
        PanelUi::RenderActionButton("开始预览", [this]() {
            LOG_INFO("Start preview button clicked");
            StartPreview();
        }, previewButtonDisabled, buttonWidth, buttonHeight);
    }
}

void CameraPanel::RenderDisplayControls(bool showResolutionSelector, bool includePreviewToggle, float actionButtonWidth, float actionButtonHeight, float previewButtonWidth, float previewButtonHeight) {
    PanelUi::RenderSectionTitle("显示控制:");

    if (includePreviewToggle) {
        RenderPreviewToggleButton(previewButtonWidth, previewButtonHeight);
        PanelUi::ContinueOnSameLineIfFits(actionButtonWidth);
        PanelUi::RenderActionButton("全屏模式", [this]() {
            ToggleFullscreen();
        }, !m_connected, actionButtonWidth, actionButtonHeight);
    }

    if (showResolutionSelector) {
        PanelUi::BeginInlineFieldRow("分辨率:", includePreviewToggle ? 0.0f : actionButtonWidth, 180.0f);
        RenderResolutionSelector();

        if (!includePreviewToggle) {
            ImGui::SameLine();
            PanelUi::RenderActionButton("全屏模式", [this]() {
                ToggleFullscreen();
            }, !m_connected, actionButtonWidth, actionButtonHeight);
        }
    }
}

void CameraPanel::RenderBasicAdjustmentControls(const char* brightnessId, const char* contrastId, const char* saturationId, bool includeSaturation) {
    if (PanelUi::RenderLabeledSliderInt("亮度:", brightnessId, &m_brightness, 0, 100)) {
        ApplyBrightnessSetting();
    }
    if (PanelUi::RenderLabeledSliderInt("对比度:", contrastId, &m_contrast, 0, 100)) {
        ApplyContrastSetting();
    }
    if (includeSaturation) {
        if (PanelUi::RenderLabeledSliderInt("饱和度:", saturationId, &m_saturation, 0, 100) && m_cameraManager) {
            if (!m_cameraManager->SetSaturation(m_saturation)) {
                LOG_ERROR("Failed to set saturation: " + m_cameraManager->GetLastError());
            }
        }
    }
}

void CameraPanel::RenderQuickAdjustmentControls(const char* brightnessId, const char* contrastId, const char* modeId) {
    RenderBasicAdjustmentControls(brightnessId, contrastId, "##unused_saturation", false);
    RenderCaptureModeSelector(modeId, "拍摄模式:", true);
    ImGui::Checkbox("显示网格线", &m_showGridLines);
}

void CameraPanel::RenderCaptureModeSelector(const char* controlId, const char* label, bool shortLabels) {
    const char* fullModes[] = { "普通模式", "文档模式", "身份证模式", "A4文档模式" };
    const char* shortModes[] = { "普通", "文档", "身份证", "A4" };
    int mode = static_cast<int>(m_captureMode);
    if (PanelUi::RenderLabeledCombo(label, controlId, &mode, shortLabels ? shortModes : fullModes, IM_ARRAYSIZE(fullModes), shortLabels ? 140.0f : 180.0f)) {
        m_captureMode = static_cast<CaptureMode>(mode);
        if (m_captureMode != CaptureMode::Document) {
            m_hasDetectedDocument = false;
            m_lastDocumentDetectionTime = std::chrono::steady_clock::time_point{};
        }
    }
}

void CameraPanel::ApplyBrightnessSetting() {
    if (m_cameraManager) {
        m_cameraManager->SetBrightness(m_brightness);
    }
}

void CameraPanel::ApplyContrastSetting() {
    if (m_cameraManager) {
        m_cameraManager->SetContrast(m_contrast);
    }
}

void CameraPanel::ResetBasicImageAdjustments() {
    m_brightness = 50;
    m_contrast = 50;
    m_saturation = 50;
    ApplyBrightnessSetting();
    ApplyContrastSetting();
}

void CameraPanel::SyncCurrentFrame(const std::vector<uint8_t>& imageData, int width, int height, Plugins::PixelFormat format) {
    m_imageData = imageData;
    m_imageWidth = width;
    m_imageHeight = height;
    m_currentFPS = std::max(1, m_currentResolution.fps);
    m_currentPixelFormat = format;
    m_currentFormat = GetPixelFormatString(format);
    m_hasImageData = UploadPreviewFrame(m_textureRenderer.get(), m_imageData, width, height, format);
}

void CameraPanel::ResetPreviewFrameState() {
    m_hasImageData = false;
    m_hasDetectedDocument = false;
    m_imageWidth = 0;
    m_imageHeight = 0;
    m_currentPixelFormat = Plugins::PixelFormat::Unknown;
    m_currentFormat = GetPixelFormatString(Plugins::PixelFormat::Unknown);
    m_imageData.clear();
    m_lastDocumentDetectionTime = std::chrono::steady_clock::time_point{};
}

void CameraPanel::StopRecordingSession() {
    if (m_recordingStartTime.time_since_epoch().count() != 0) {
        m_lastRecordingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_recordingStartTime);
    }
    m_lastRecordedFrameCount = std::max(m_lastRecordedFrameCount, m_recordingFrameIndex);
    m_recording = false;
    m_lastRecordedFrameTime = std::chrono::steady_clock::time_point{};
}

bool CameraPanel::UpdateDocumentDetectionFromFrame(const std::vector<uint8_t>& imageData, int width, int height, Plugins::PixelFormat format, bool logResult) {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    if (DetectDocumentBounds(imageData, width, height, format, m_edgeThreshold, left, top, right, bottom)) {
        m_hasDetectedDocument = true;
        m_detectedDocumentLeft = left;
        m_detectedDocumentTop = top;
        m_detectedDocumentRight = right;
        m_detectedDocumentBottom = bottom;
        if (logResult) {
            LOG_INFO("Document edges detected successfully");
        }
        return true;
    }

    m_hasDetectedDocument = false;
    if (logResult) {
        LOG_WARNING("No document edges detected in current frame");
    }
    return false;
}

bool CameraPanel::OpenRecordingOutputDirectory() const {
    if (!DirectoryContainsRecordingFrames(m_recordingOutputDir)) {
        return false;
    }

#ifdef _WIN32
    const HINSTANCE result = ShellExecuteA(nullptr, "open", m_recordingOutputDir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(result) > 32;
#else
    const std::string command = std::string("xdg-open ") + QuoteCommandArgument(m_recordingOutputDir) + " >/dev/null 2>&1";
    return std::system(command.c_str()) == 0;
#endif
}

bool CameraPanel::ExportRecordingToVideo() {
    if (m_recording || !DirectoryContainsRecordingFrames(m_recordingOutputDir)) {
        m_videoExportStatus = "无可导出的帧序列";
        return false;
    }

#ifdef _WIN32
    if (std::system("ffmpeg -version >nul 2>&1") != 0) {
        m_videoExportStatus = "导出失败：未找到 ffmpeg";
        return false;
    }
#else
    if (std::system("ffmpeg -version >/dev/null 2>&1") != 0) {
        m_videoExportStatus = "导出失败：未找到 ffmpeg";
        return false;
    }
#endif

    const std::filesystem::path outputDirectory(m_recordingOutputDir);
    const std::filesystem::path inputPattern = outputDirectory / "frame_%06d.bmp";
    const std::filesystem::path outputVideo = outputDirectory / ("recording_" + BuildCameraRecordingTimestamp() + ".mp4");
    const int exportFps = std::max(1, m_recordingFPS);

    std::ostringstream command;
    command << "ffmpeg -y -framerate " << exportFps
            << " -i " << QuoteCommandArgument(inputPattern.generic_string())
            << " -c:v libx264 -pix_fmt yuv420p "
            << QuoteCommandArgument(outputVideo.generic_string());
#ifdef _WIN32
    command << " >nul 2>&1";
#else
    command << " >/dev/null 2>&1";
#endif

    if (std::system(command.str().c_str()) != 0) {
        m_videoExportStatus = "导出失败：ffmpeg 执行错误";
        return false;
    }

    m_lastExportedVideoPath = outputVideo.string();
    m_videoExportStatus = "导出成功";
    return true;
}

std::string CameraPanel::GetRecordingDurationText() const {
    std::chrono::milliseconds duration = m_lastRecordingDuration;
    if (m_recording && m_recordingStartTime.time_since_epoch().count() != 0) {
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_recordingStartTime);
    }

    const long long totalSeconds = duration.count() / 1000;
    const long long hours = totalSeconds / 3600;
    const long long minutes = (totalSeconds % 3600) / 60;
    const long long seconds = totalSeconds % 60;

    std::ostringstream stream;
    stream << std::setw(2) << std::setfill('0') << hours << ':'
           << std::setw(2) << std::setfill('0') << minutes << ':'
           << std::setw(2) << std::setfill('0') << seconds;
    return stream.str();
}

// 辅助函数：渲染预览占位符
static void RenderPreviewPlaceholder(const ImVec2& startPos, const ImVec2& size,
                                   const char* mainText, const char* hintText, bool isError = false) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 previewStart = ImVec2(windowPos.x + startPos.x, windowPos.y + startPos.y);
    ImVec2 previewEnd = ImVec2(previewStart.x + size.x, previewStart.y + size.y);
    
    // 绘制背景
    ImU32 bgColor = isError ? IM_COL32(50, 30, 30, 255) : IM_COL32(40, 40, 40, 255);
    ImU32 borderColor = IM_COL32(100, 100, 100, 255);
    drawList->AddRectFilled(previewStart, previewEnd, bgColor);
    drawList->AddRect(previewStart, previewEnd, borderColor, 0.0f, 0, 2.0f);
    
    // 绘制中心内容
    ImVec2 center = ImVec2((previewStart.x + previewEnd.x) * 0.5f, (previewStart.y + previewEnd.y) * 0.5f);
    
    // 摄像头图标
    ImVec2 iconPos = ImVec2(center.x - 30, center.y - 50);
    drawList->AddText(iconPos, IM_COL32(80, 80, 80, 255), "CAM");
    
    // 主文本
    ImVec2 textPos = ImVec2(center.x - (mainText ? strlen(mainText) * 3.5f : 50), center.y + 5);
    ImU32 textColor = isError ? IM_COL32(200, 100, 100, 255) : IM_COL32(160, 160, 160, 255);
    drawList->AddText(textPos, textColor, mainText);
    
    // 提示文本
    if (hintText) {
        ImVec2 hintPos = ImVec2(center.x - strlen(hintText) * 3.5f, center.y + 25);
        drawList->AddText(hintPos, IM_COL32(120, 120, 120, 255), hintText);
    }
}

static bool UploadPreviewFrame(TextureRenderer* textureRenderer,
                                const std::vector<uint8_t>& imageData,
                                int width,
                                int height,
                                Plugins::PixelFormat format) {
    if (!textureRenderer || imageData.empty()) {
        return false;
    }

    switch (format) {
        case Plugins::PixelFormat::RGB24:
            return textureRenderer->UploadRGB(imageData.data(), width, height);
        case Plugins::PixelFormat::BGR24:
            return textureRenderer->UploadBGR(imageData.data(), width, height);
        case Plugins::PixelFormat::RGB32:
            return textureRenderer->UploadRGBA(imageData.data(), width, height);
        default:
            return false;
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
    
    // 扩展窗口边界（防止断言错误）
    ImGui::Dummy(previewSize);
    ImGui::SetCursorPos(centerPos);
    
    // 显示预览图像
    if (m_previewActive && m_connected) {
        // 获取最新的图像数据
        if (m_cameraManager) {
            Plugins::PixelFormat format;
            int width, height;
            if (m_cameraManager->CaptureImage(m_imageData, format, width, height)) {
                SyncCurrentFrame(m_imageData, width, height, format);
                if (m_recording && m_hasImageData && !m_recordingOutputDir.empty()) {
                    const auto now = std::chrono::steady_clock::now();
                    const int targetFps = std::max(1, m_currentFPS);
                    const auto frameInterval = std::chrono::milliseconds(1000 / targetFps);
                    if (m_lastRecordedFrameTime.time_since_epoch().count() == 0 || (now - m_lastRecordedFrameTime) >= frameInterval) {
                        std::ostringstream fileName;
                        fileName << "frame_" << std::setw(6) << std::setfill('0') << m_recordingFrameIndex << ".bmp";
                        const std::filesystem::path framePath = std::filesystem::path(m_recordingOutputDir) / fileName.str();
                        if (WriteBmpFrame(framePath.string(), m_imageData, width, height, format)) {
                            ++m_recordingFrameIndex;
                            m_lastRecordedFrameCount = m_recordingFrameIndex;
                            m_lastRecordingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_recordingStartTime);
                            m_lastRecordedFrameTime = now;
                        } else {
                            LOG_ERROR("Failed to write recording frame: " + framePath.string());
                            StopRecordingSession();
                        }
                    }
                }

                if (m_captureMode == CaptureMode::Document && m_autoDetectDocument && m_hasImageData) {
                    const auto now = std::chrono::steady_clock::now();
                    const auto detectInterval = std::chrono::milliseconds(std::max(100, m_autoDetectIntervalMs));
                    if (m_lastDocumentDetectionTime.time_since_epoch().count() == 0 || (now - m_lastDocumentDetectionTime) >= detectInterval) {
                        UpdateDocumentDetectionFromFrame(m_imageData, width, height, format, false);
                        m_lastDocumentDetectionTime = now;
                    }
                }
            } else {
                m_hasImageData = false;
            }
        }
        
        if (m_hasImageData && m_textureRenderer && m_textureRenderer->HasValidTexture()) {
            // 使用 ImGui::Image 直接渲染纹理（GPU加速，高效）
            ImGui::Image(m_textureRenderer->GetImTextureID(), previewSize);
        } else {
            RenderPreviewPlaceholder(centerPos, previewSize, "正在获取图像...", "请稍候，正在连接摄像头");
        }
    } else {
        // 非预览状态下的占位符
        const char* mainText = !m_previewActive ? "摄像头预览" : "设备未连接";
        const char* hintText = m_connected ? "点击 [开始预览] 按钮开始" : "请先选择并连接摄像头设备";
        bool isError = !m_connected;
        
        RenderPreviewPlaceholder(centerPos, previewSize, mainText, hintText, isError);
    }
    
    // 双击全屏
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        ToggleFullscreen();
    }
    
    // 显示网格线
    if (m_showGridLines) {
        RenderGridLines(centerPos, previewSize);
    }

    if (m_captureMode == CaptureMode::Document && m_previewActive && m_hasImageData) {
        RenderDocumentBoundsOverlay(centerPos, previewSize, m_hasDetectedDocument, m_detectedDocumentLeft, m_detectedDocumentTop, m_detectedDocumentRight, m_detectedDocumentBottom);
    }
}

void CameraPanel::RenderFullscreenPreview() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->Size, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("CameraFullscreenPreview", nullptr, flags)) {
        const float buttonWidth = 120.0f;
        ImGui::TextUnformatted("全屏预览");
        const float closeButtonX = ImGui::GetWindowContentRegionMax().x - buttonWidth;
        if (closeButtonX > ImGui::GetCursorPosX()) {
            ImGui::SameLine(closeButtonX);
        }
        if (ImGui::Button("退出全屏", ImVec2(buttonWidth, 30.0f))) {
            ExitFullscreen();
        }

        ImGui::Separator();

        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        ImVec2 previewSize = CalculatePreviewSize(availableSize, m_currentResolution);
        ImVec2 origin = ImGui::GetCursorPos();
        ImVec2 centerPos = ImVec2(
            origin.x + (availableSize.x - previewSize.x) * 0.5f,
            origin.y + (availableSize.y - previewSize.y) * 0.5f
        );
        ImGui::SetCursorPos(centerPos);
        ImGui::Dummy(previewSize);
        ImGui::SetCursorPos(centerPos);

        if (m_previewActive && m_connected) {
            if (m_hasImageData && m_textureRenderer && m_textureRenderer->HasValidTexture()) {
                ImGui::Image(m_textureRenderer->GetImTextureID(), previewSize);
            } else {
                RenderPreviewPlaceholder(centerPos, previewSize, "正在获取图像...", "请稍候，正在连接摄像头");
            }
        } else {
            const char* mainText = !m_previewActive ? "摄像头预览" : "设备未连接";
            const char* hintText = m_connected ? "点击 [开始预览] 按钮开始" : "请先选择并连接摄像头设备";
            const bool isError = !m_connected;
            
            RenderPreviewPlaceholder(centerPos, previewSize, mainText, hintText, isError);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            ExitFullscreen();
        }

        if (m_showGridLines) {
            RenderGridLines(centerPos, previewSize);
        }

        if (m_captureMode == CaptureMode::Document && m_previewActive && m_hasImageData) {
            RenderDocumentBoundsOverlay(centerPos, previewSize, m_hasDetectedDocument, m_detectedDocumentLeft, m_detectedDocumentTop, m_detectedDocumentRight, m_detectedDocumentBottom);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void CameraPanel::RenderControlPanel() {
    PanelUi::RenderSectionTitle("主要操作:");
    
    // 大号拍照按钮（类似真实相机）
    RenderCaptureActionButtons(80.0f, 80.0f);
    
    RenderDisplayControls(true, true, 120.0f, 30.0f, 120.0f, 30.0f);
    
    PanelUi::RenderSectionTitle("画质调节:");
    RenderBasicAdjustmentControls("##brightness_panel", "##contrast_panel", "##saturation_panel", true);
    
    // 重置按钮
    PanelUi::RenderActionButton("重置设置", [this]() {
        ResetBasicImageAdjustments();
    }, false, 120.0f, 25.0f);
    
    RenderDocumentMode();
    RenderImageEnhancement();
}

void CameraPanel::RenderDocumentMode() {
    PanelUi::RenderSectionTitle("拍摄模式:");
    
    RenderCaptureModeSelector("##document_mode_panel", "模式:", false);
    
    if (m_captureMode == CaptureMode::Document) {
        PanelUi::RenderActionButton("立即检测边缘", [this]() {
            DetectDocumentEdges();
        }, !m_connected || !m_previewActive);

        ImGui::Checkbox("连续自动检测", &m_autoDetectDocument);
        if (m_autoDetectDocument) {
            PanelUi::RenderLabeledSliderInt("检测间隔(ms):", "##auto_detect_interval_panel", &m_autoDetectIntervalMs, 100, 2000);
        }
        
        PanelUi::RenderLabeledSliderFloat("边缘阈值:", "##edge_threshold_panel", &m_edgeThreshold, 0.1f, 1.0f);
    }
    
    // 网格线选项
    ImGui::Checkbox("显示网格线", &m_showGridLines);
}

void CameraPanel::RenderImageEnhancement() {
    PanelUi::RenderSectionTitle("图像增强:");
    
    ImGui::Checkbox("自动增强", &m_autoEnhance);
    
    if (!m_autoEnhance) {
        PanelUi::RenderLabeledSliderInt("锐化:", "##sharpness_panel", &m_sharpness, 0, 100);
        PanelUi::RenderLabeledSliderInt("降噪:", "##denoise_panel", &m_denoise, 0, 100);
        ImGui::Checkbox("自动旋转", &m_autoRotate);
    }
}

void CameraPanel::RenderPreviewOverlay() {
    // 移除预览区域的信息覆盖层，避免与占位背景图重叠
}

void CameraPanel::RenderImageInfo() {
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    float availableHeight = availableSize.y;

    PanelUi::RenderSectionTitle("图像信息:");

    std::string resolutionText;
    if (m_connected && m_hasImageData && m_imageWidth > 0 && m_imageHeight > 0) {
        resolutionText = std::to_string(m_imageWidth) + "x" + std::to_string(m_imageHeight);
    } else if (m_connected) {
        resolutionText = std::to_string(m_currentResolution.width) + "x" + std::to_string(m_currentResolution.height);
    } else {
        resolutionText = "未检测";
    }

    std::string fpsText = std::to_string(m_currentFPS) + " fps";
    const char* captureModes[] = { "普通", "文档", "身份证", "A4" };
    const std::string recordingText = m_recording ? "录制中(帧序列)" : (m_recordingOutputDir.empty() ? "未录像" : "已保存帧序列");
    const int recordedFrameCount = m_recording ? m_recordingFrameIndex : m_lastRecordedFrameCount;
    const std::string frameCountText = std::to_string(recordedFrameCount) + " 帧";
    const std::string recordingDurationText = GetRecordingDurationText();
    ImVec4 previewStatusColor = m_previewActive ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    PanelUi::RenderStatusSummary("预览状态:", m_previewActive ? "预览中" : "已停止", previewStatusColor);
    PanelUi::RenderResponsiveInfoFields(
        "CameraInfoColumns",
        std::array<PanelUi::InfoField, 4>{{
            {"分辨率:", resolutionText.c_str()},
            {"帧率:", fpsText.c_str()},
            {"格式:", m_currentFormat.c_str()},
            {"拍摄模式:", captureModes[static_cast<int>(m_captureMode)]}
        }},
        std::array<PanelUi::InfoField, 4>{{
            {"录像状态:", recordingText.c_str()},
            {"录像时长:", recordingDurationText.c_str()},
            {"录制帧数:", frameCountText.c_str()},
            {"导出状态:", m_videoExportStatus.c_str()}
        }}
    );
    if (!m_recordingOutputDir.empty()) {
        PanelUi::RenderInfoField({"输出目录:", m_recordingOutputDir.c_str()});
    }
    if (!m_lastExportedVideoPath.empty()) {
        PanelUi::RenderInfoField({"导出文件:", m_lastExportedVideoPath.c_str()});
    }
    if (m_captureMode == CaptureMode::Document) {
        PanelUi::RenderInfoField({"文档检测:", m_hasDetectedDocument ? "已检测" : "未检测"});
    }

    const bool hasRecordingFrames = DirectoryContainsRecordingFrames(m_recordingOutputDir) && recordedFrameCount > 0;
    PanelUi::RenderActionButton("打开录制目录", [this]() {
        if (!OpenRecordingOutputDirectory()) {
            LOG_ERROR("Failed to open recording output directory: " + m_recordingOutputDir);
        }
    }, !hasRecordingFrames, 120.0f, 28.0f);
    PanelUi::ContinueOnSameLineIfFits(120.0f);
    PanelUi::RenderActionButton("导出视频", [this]() {
        if (!ExportRecordingToVideo()) {
            LOG_WARNING("Recording export did not complete successfully");
        }
    }, !hasRecordingFrames || m_recording, 120.0f, 28.0f);

    if (availableHeight > 60) {
        PanelUi::RenderSectionTitle("快速调整:");
        RenderQuickAdjustmentControls("##brightness_quick", "##contrast_quick", "##mode_quick");
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
        SyncCurrentFrame(imageData, width, height, format);
        LOG_INFO("Image captured successfully");
    } else {
        LOG_ERROR("Failed to capture image: " + m_cameraManager->GetLastError());
    }
}

void CameraPanel::ToggleRecording() {
    if (!m_connected || !m_previewActive) {
        LOG_WARNING("Camera preview is not active, cannot toggle recording");
        return;
    }

    if (m_recording) {
        StopRecordingSession();
        LOG_INFO("Recording stopped, frames saved to: " + m_recordingOutputDir);
        return;
    }

    try {
        std::filesystem::path outputDir = std::filesystem::current_path() / "recordings" / ("camera_" + BuildCameraRecordingTimestamp());
        std::filesystem::create_directories(outputDir);
        m_recordingOutputDir = outputDir.string();
        m_recordingFrameIndex = 0;
        m_lastRecordedFrameCount = 0;
        m_lastRecordedFrameTime = std::chrono::steady_clock::time_point{};
        m_recordingStartTime = std::chrono::steady_clock::now();
        m_lastRecordingDuration = std::chrono::milliseconds{0};
        m_recordingFPS = std::max(1, m_currentFPS);
        m_lastExportedVideoPath.clear();
        m_videoExportStatus = "未导出";
        m_recording = true;
        LOG_INFO("Recording started, output directory: " + m_recordingOutputDir);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to start recording: " + std::string(e.what()));
        m_recording = false;
    }
}

void CameraPanel::ToggleFullscreen() {
    if (m_fullscreen) {
        ExitFullscreen();
        return;
    }

    if (!m_connected) {
        LOG_WARNING("Camera not connected, cannot enter fullscreen preview");
        return;
    }

    m_fullscreen = true;
    LOG_INFO("Entered fullscreen mode");
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
        if (m_availableCameras.empty()) {
            m_selectedDevice = 0;
        } else if (m_selectedDevice >= static_cast<int>(m_availableCameras.size())) {
            m_selectedDevice = 0;
        }
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

    std::vector<uint8_t> sourceImage = m_imageData;
    Plugins::PixelFormat sourceFormat = m_currentPixelFormat;
    int sourceWidth = m_imageWidth;
    int sourceHeight = m_imageHeight;

    if ((!m_hasImageData || sourceImage.empty() || sourceWidth <= 0 || sourceHeight <= 0) && m_cameraManager && m_connected) {
        if (!m_cameraManager->CaptureImage(sourceImage, sourceFormat, sourceWidth, sourceHeight)) {
            LOG_ERROR("Failed to capture image for document detection: " + m_cameraManager->GetLastError());
            m_hasDetectedDocument = false;
            return;
        }
    }

    SyncCurrentFrame(sourceImage, sourceWidth, sourceHeight, sourceFormat);
    UpdateDocumentDetectionFromFrame(sourceImage, sourceWidth, sourceHeight, sourceFormat, true);
    m_lastDocumentDetectionTime = std::chrono::steady_clock::now();
}

bool CameraPanel::StartPreview() {
    if (!m_cameraManager || !m_connected) {
        LOG_WARNING("Camera not connected, cannot start preview");
        return false;
    }

    if (m_previewActive) {
        return true;
    }
    
    if (m_cameraManager->StartPreview()) {
        m_previewActive = true;
        m_lastDocumentDetectionTime = std::chrono::steady_clock::time_point{};
        LOG_INFO("Preview started");
        return true;
    } else {
        LOG_ERROR("Failed to start preview: " + m_cameraManager->GetLastError());
        return false;
    }
}

bool CameraPanel::StopPreview() {
    if (!m_previewActive) {
        StopRecordingSession();
        ResetPreviewFrameState();
        return true;
    }

    if (!m_cameraManager) {
        LOG_WARNING("Camera manager not available, cannot stop preview");
        return false;
    }

    if (!m_cameraManager->StopPreview()) {
        LOG_ERROR("Failed to stop preview: " + m_cameraManager->GetLastError());
        return false;
    }

    m_previewActive = false;
    StopRecordingSession();
    ResetPreviewFrameState();
    LOG_INFO("Preview stopped");
    return true;
}

void CameraPanel::RenderResolutionSelector() {
    // 获取当前设备支持的分辨率
    static int selectedResolution = 0;
    static std::vector<std::string> resolutionStrings;
    static std::vector<Plugins::Resolution> availableResolutions;
    
    // 如果设备已连接，获取支持的分辨率
    if (m_connected && m_cameraManager && !m_availableCameras.empty() && m_selectedDevice >= 0 && m_selectedDevice < static_cast<int>(m_availableCameras.size())) {
        availableResolutions = m_availableCameras[m_selectedDevice].supportedResolutions;
        if (availableResolutions.empty()) {
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
        for (size_t i = 0; i < availableResolutions.size(); i++) {
            if (availableResolutions[i].width == m_currentResolution.width &&
                availableResolutions[i].height == m_currentResolution.height &&
                availableResolutions[i].fps == m_currentResolution.fps) {
                selectedResolution = static_cast<int>(i);
                break;
            }
        }
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

                // 如果摄像头已连接，应用新的分辨率
                if (m_cameraManager && m_connected) {
                    const Plugins::Resolution previousResolution = m_currentResolution;
                    // 如果正在预览，需要重新启动预览以应用新分辨率
                    bool wasPreviewing = m_previewActive;
                    if (wasPreviewing && !StopPreview()) {
                        return;
                    }
                    
                    // 设置新分辨率
                    if (m_cameraManager->SetResolution(newResolution.width, newResolution.height, newResolution.fps)) {
                        m_currentResolution = newResolution;
                        m_currentFPS = newResolution.fps;
                        LOG_INFO("Resolution set to: " + std::to_string(m_currentResolution.width) + "x" + std::to_string(m_currentResolution.height) + "@" + std::to_string(m_currentResolution.fps));
                        
                        // 如果之前正在预览，重新启动预览
                        if (wasPreviewing && !StartPreview()) {
                            LOG_ERROR("Failed to restart preview after resolution change");
                        }
                    } else {
                        LOG_ERROR("Failed to set resolution: " + m_cameraManager->GetLastError());
                        m_currentResolution = previousResolution;
                        if (wasPreviewing) {
                            StartPreview();
                        }
                    }
                } else {
                    m_currentResolution = newResolution;
                    m_currentFPS = newResolution.fps;
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
        case Plugins::PixelFormat::BGR24:
            return "BGR24";
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
