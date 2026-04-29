#include "gui/SignaturePanel.h"
#include "gui/PanelUiHelpers.h"
#include "core/DeviceFactory.h"
#include "utils/Logger.h"
#include "utils/FileDialog.h"
#include "core/DeviceManager.h"
#include "imgui.h"

#ifdef PLATFORM_WINDOWS
#include "devices/signature/WindowsSignaturePad.h"
#endif

namespace AsTestTool {

namespace {

bool OpenPanelSignatureDevice(ISignaturePad* signaturePad) {
    if (!signaturePad) {
        return false;
    }

#ifdef PLATFORM_WINDOWS
    if (auto* windowsSignaturePad = dynamic_cast<WindowsSignaturePad*>(signaturePad)) {
        return windowsSignaturePad->OpenDevice();
    }
#endif

    return true;
}

bool LoadPanelSignatureLibrary(ISignaturePad* signaturePad, const std::string& libraryPath) {
#ifdef PLATFORM_WINDOWS
    if (auto* windowsSignaturePad = dynamic_cast<WindowsSignaturePad*>(signaturePad)) {
        return windowsSignaturePad->LoadLibrary(libraryPath);
    }
#endif

    LOG_ERROR("Custom signature library reload is not supported on this platform");
    return false;
}

}

SignaturePanel::SignaturePanel() {
    LOG_INFO("SignaturePanel created");
}

SignaturePanel::~SignaturePanel() {
    LOG_INFO("SignaturePanel destroyed");
}

void SignaturePanel::Render() {
    RenderDeviceStatus();
    RenderControls();
    RenderSignature();
    
    if (m_showLibrarySettings) {
        RenderLibrarySettings();
    }
}

void SignaturePanel::SetSignaturePad(ISignaturePad* signaturePad) {
    m_customSignaturePad.reset();
    m_signaturePad = signaturePad;
    LOG_INFO("Signature pad set to panel");
}

void SignaturePanel::RenderDeviceStatus() {
    std::string statusText = GetDeviceStatusText();
    ImVec4 statusColor = GetDeviceStatusColor();
    const std::string deviceInfo = m_signaturePad ? m_signaturePad->GetDeviceInfo() : std::string();
    PanelUi::RenderDeviceStatusBlock(statusText.c_str(), statusColor, deviceInfo.c_str());
}

void SignaturePanel::RenderSignature() {
    PanelUi::RenderSectionTitle("手写轨迹:");
    bool hasValidSignature = m_hasSignature && m_currentSignature.HasData();
    ImVec4 captureStatusColor = hasValidSignature ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    PanelUi::RenderStatusSummary("捕获状态:", hasValidSignature ? "已捕获" : "未捕获", captureStatusColor);
    
    // 手写区域
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImVec2 signatureSize = ImVec2(std::max(320.0f, availableSize.x), 200.0f);
    ImGui::BeginChild("Signature", signatureSize, true, ImGuiWindowFlags_NoScrollbar);
    
    if (hasValidSignature) {
        PanelUi::RenderStateBanner("=== 手写数据已捕获 ===", captureStatusColor);
        const std::string pointCountText = std::to_string(m_currentSignature.GetPointCount());
        const std::string screenSizeText = std::to_string(m_currentSignature.width) + "x" + std::to_string(m_currentSignature.height);
        PanelUi::RenderResponsiveInfoFields(
            "SignatureInfoColumns",
            std::array<PanelUi::InfoField, 2>{{
                {"轨迹点数:", pointCountText.c_str()},
                {"屏幕尺寸:", screenSizeText.c_str()}
            }},
            std::array<PanelUi::InfoField, 1>{{
                {"设备信息:", m_currentSignature.deviceInfo.c_str()}
            }}
        );
        
        // 显示轨迹预览（简化版本）
        PanelUi::RenderSectionTitle("轨迹预览:");
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        float canvasWidth = ImGui::GetContentRegionAvail().x;
        if (canvasWidth < 220.0f) {
            canvasWidth = 220.0f;
        }
        if (canvasWidth > 350.0f) {
            canvasWidth = 350.0f;
        }
        ImVec2 canvasSize = ImVec2(canvasWidth, 100.0f);
        
        // 绘制边框
        drawList->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(255, 255, 255, 255));
        
        // 绘制轨迹点
        if (m_currentSignature.GetPointCount() > 0) {
            for (size_t i = 0; i < m_currentSignature.GetPointCount() && i < 100; ++i) {
                const auto& point = m_currentSignature.points[i];
                ImVec2 pos = ImVec2(
                    canvasPos.x + point.x * canvasSize.x,
                    canvasPos.y + point.y * canvasSize.y
                );
                
                // 根据压力值设置颜色
                int alpha = (int)(point.pressure * 255);
                ImU32 color = IM_COL32(255, 255, 255, alpha);
                
                if (point.isDown) {
                    drawList->AddCircleFilled(pos, 2.0f, color);
                }
            }
        }
        
        ImGui::Dummy(canvasSize);
    } else {
        PanelUi::RenderStateBanner("=== 手写数据未捕获 ===", captureStatusColor);
        ImGui::Text("轨迹点数: 0");
        ImGui::Text("屏幕尺寸: 未获取");
        ImGui::Text("请先连接设备并开始捕获");
    }
    
    ImGui::EndChild();
}

void SignaturePanel::RenderControls() {
    PanelUi::RenderSectionTitle("主要操作:");
    PanelUi::RenderConnectionToggleButton(
        IsDeviceConnected(),
        [this]() { ConnectDevice(); },
        [this]() { DisconnectDevice(); }
    );
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    PanelUi::RenderActionButton(
        m_signaturePad && m_signaturePad->IsCapturing() ? "停止捕获" : "开始捕获",
        [this]() {
            if (m_signaturePad && m_signaturePad->IsCapturing()) {
                StopCapture();
            } else {
                StartCapture();
            }
        },
        !IsDeviceConnected()
    );
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    PanelUi::RenderActionButton("清除轨迹", [this]() { ClearSignature(); }, !IsDeviceConnected());
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    PanelUi::RenderActionButton("获取数据", [this]() {
        if (m_signaturePad) {
            SignatureData data;
            if (m_signaturePad->GetSignatureData(data)) {
                m_currentSignature = data;
                m_hasSignature = true;
                LOG_INFO("Signature data retrieved successfully");
            } else {
                LOG_WARNING("No signature data available");
                m_currentSignature.Clear();
                m_hasSignature = false;
            }
        }
    }, !IsDeviceConnected());
    
    PanelUi::RenderSectionTitle("显示控制:");
    
    // 全屏模式控制
    PanelUi::RenderActionButton("全屏模式", []() {}, true);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("手写屏全屏模式当前尚未提供真实驱动支持");
    }
    
    PanelUi::RenderSectionTitle("库管理:");
    if (ImGui::Button("库设置", ImVec2(PanelUi::kButtonWidth, PanelUi::kButtonHeight))) {
        m_showLibrarySettings = !m_showLibrarySettings;
    }
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    PanelUi::RenderActionButton("重新加载库", [this]() {
        LoadSignatureLibrary();
    });
}

void SignaturePanel::RenderLibrarySettings() {
    PanelUi::PrepareLibrarySettingsWindow();
    if (ImGui::Begin("手写屏库设置", &m_showLibrarySettings)) {
        PanelUi::RenderSectionTitle("基础配置:");
        
        char pathBuffer[512];
        strncpy_s(pathBuffer, m_libraryPath.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        
        if (PanelUi::RenderPathSelectorRow("库文件路径:", "##LibraryPath", pathBuffer, sizeof(pathBuffer), [this]() {
            auto fileDialog = FileDialog::Create();
            if (fileDialog) {
                std::vector<FileDialogFilter> filters = {
                    {"动态链接库 (*.dll)", "*.dll"},
                    {"所有文件 (*.*)", "*.*"}
                };
                auto result = fileDialog->OpenFile("选择手写屏DLL文件", filters);
                if (result.success) {
                    m_libraryPath = result.filePath;
                    LOG_INFO("Selected DLL file: " + m_libraryPath);
                }
            }
        })) {
            m_libraryPath = std::string(pathBuffer);
        }
        
        PanelUi::RenderDefaultLibrarySection("CMCC_SIGN.DLL");
        
        PanelUi::RenderDescriptionSection({
            "- 库文件路径: 手写屏DLL文件路径",
            "- 支持的库: cmcc_sign.dll 等标准接口库",
            "- 修改后可点击 [应用设置] 重新记录配置"
        });

        PanelUi::RenderSettingsActionSection(&m_showLibrarySettings, [this]() {
            LoadSignatureLibrary();
        });
    }
    ImGui::End();
}

// 设备控制方法实现
void SignaturePanel::ConnectDevice() {
    LOG_INFO("Connecting signature pad device");
    
    if (m_customSignaturePad) {
        m_signaturePad = m_customSignaturePad.get();
    } else {
        m_signaturePad = DeviceManager::Instance().GetSignaturePad();
    }
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    if (!m_signaturePad->Initialize()) {
        LOG_ERROR("Failed to connect signature pad device");
        return;
    }

    if (!OpenPanelSignatureDevice(m_signaturePad)) {
        LOG_ERROR("Failed to open signature pad device");
        return;
    }

    LOG_INFO("Signature pad device connected successfully");
}

void SignaturePanel::DisconnectDevice() {
    LOG_INFO("Disconnecting signature pad device");
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    // 停止捕获
    StopCapture();
    
    m_signaturePad->Shutdown();
    m_currentSignature.Clear();
    m_hasSignature = false;
    m_fullscreen = false;
    LOG_INFO("Signature pad device disconnected successfully");
}

void SignaturePanel::StartCapture() {
    LOG_INFO("Starting signature capture");
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    if (m_signaturePad->StartCapture()) {
        LOG_INFO("Signature capture started successfully");
    } else {
        LOG_ERROR("Failed to start signature capture");
    }
}

void SignaturePanel::StopCapture() {
    LOG_INFO("Stopping signature capture");
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    if (m_signaturePad->StopCapture()) {
        LOG_INFO("Signature capture stopped successfully");
    } else {
        LOG_ERROR("Failed to stop signature capture");
    }
}

void SignaturePanel::ClearSignature() {
    LOG_INFO("Clearing signature");
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    if (m_signaturePad->ClearSignature()) {
        m_currentSignature.Clear();
        m_hasSignature = false;
        LOG_INFO("Signature cleared successfully");
    } else {
        LOG_ERROR("Failed to clear signature");
    }
}

void SignaturePanel::LoadSignatureLibrary() {
    LOG_INFO("Loading signature pad library: " + m_libraryPath);

    if (m_signaturePad && IsDeviceConnected()) {
        DisconnectDevice();
    }

    auto customSignaturePad = DeviceFactory::CreateSignaturePad("custom");
    if (!customSignaturePad) {
        LOG_ERROR("Failed to create custom signature pad instance");
        return;
    }

    if (!customSignaturePad->Initialize()) {
        LOG_ERROR("Failed to initialize custom signature pad instance");
        return;
    }

    if (!LoadPanelSignatureLibrary(customSignaturePad.get(), m_libraryPath)) {
        customSignaturePad->Shutdown();
        return;
    }

    if (!OpenPanelSignatureDevice(customSignaturePad.get())) {
        LOG_ERROR("Failed to open signature pad after library reload");
        customSignaturePad->Shutdown();
        return;
    }

    m_customSignaturePad = std::move(customSignaturePad);
    m_signaturePad = m_customSignaturePad.get();
    m_currentSignature.Clear();
    m_hasSignature = false;
    m_fullscreen = false;
    LOG_INFO("Signature pad library reloaded successfully: " + m_libraryPath);
}

// 状态检查方法实现
bool SignaturePanel::IsDeviceConnected() const {
    return m_signaturePad && m_signaturePad->IsConnected();
}

std::string SignaturePanel::GetDeviceStatusText() const {
    if (!m_signaturePad) {
        return "设备未设置";
    }
    
    if (IsDeviceConnected()) {
        return "已连接";
    } else {
        return "未连接";
    }
}

ImVec4 SignaturePanel::GetDeviceStatusColor() const {
    if (!m_signaturePad) {
        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }

    if (IsDeviceConnected()) {
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 绿色
    } else {
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 红色
    }
}

} // namespace AsTestTool
