#include "gui/SignaturePanel.h"
#include "utils/Logger.h"
#include "utils/FileDialog.h"
#include "core/DeviceManager.h"
#include "imgui.h"

namespace {
constexpr float kPanelButtonWidth = 100.0f;
constexpr float kPanelButtonHeight = 30.0f;

void ContinueOnSameLineIfFits(float nextItemWidth) {
    const ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetContentRegionAvail().x >= nextItemWidth + style.ItemSpacing.x) {
        ImGui::SameLine();
    }
}
}

namespace AsTestTool {

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

void SignaturePanel::SetSignaturePad(std::shared_ptr<ISignaturePad> signaturePad) {
    m_signaturePad = signaturePad;
    LOG_INFO("Signature pad set to panel");
}

void SignaturePanel::RenderDeviceStatus() {
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    
    std::string statusText = GetDeviceStatusText();
    ImVec4 statusColor = GetDeviceStatusColor();
    ImGui::TextColored(statusColor, "%s", statusText.c_str());
    
    if (m_signaturePad) {
        std::string deviceInfo = m_signaturePad->GetDeviceInfo();
        if (!deviceInfo.empty()) {
            ImGui::Text("设备信息: %s", deviceInfo.c_str());
        }
    }
    
    ImGui::Separator();
}

void SignaturePanel::RenderSignature() {
    ImGui::Text("手写轨迹:");
    ImGui::Separator();
    bool hasValidSignature = m_hasSignature && m_currentSignature.HasData();
    ImVec4 captureStatusColor = hasValidSignature ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImGui::Text("捕获状态:");
    ImGui::SameLine();
    ImGui::TextColored(captureStatusColor, "%s", hasValidSignature ? "已捕获" : "未捕获");
    
    // 手写区域
    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    ImVec2 signatureSize = ImVec2(std::max(320.0f, availableSize.x), 200.0f);
    ImGui::BeginChild("Signature", signatureSize, true, ImGuiWindowFlags_NoScrollbar);
    
    if (hasValidSignature) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== 手写数据已捕获 ===");
        bool useDenseLayout = ImGui::GetContentRegionAvail().x >= 420.0f;
        if (useDenseLayout) {
            ImGui::Columns(2, "SignatureInfoColumns", false);
            ImGui::Text("轨迹点数: %zu", m_currentSignature.GetPointCount());
            ImGui::NextColumn();
            ImGui::Text("屏幕尺寸: %dx%d", m_currentSignature.width, m_currentSignature.height);
            ImGui::Columns(1);
            ImGui::Text("设备信息: %s", m_currentSignature.deviceInfo.c_str());
        } else {
            ImGui::Text("轨迹点数: %zu", m_currentSignature.GetPointCount());
            ImGui::Text("屏幕尺寸: %dx%d", m_currentSignature.width, m_currentSignature.height);
            ImGui::Text("设备信息: %s", m_currentSignature.deviceInfo.c_str());
        }
        
        // 显示轨迹预览（简化版本）
        ImGui::Separator();
        ImGui::Text("轨迹预览:");
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImVec2(350, 100);
        
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
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "=== 手写数据未捕获 ===");
        ImGui::Text("轨迹点数: 0");
        ImGui::Text("屏幕尺寸: 未获取");
        ImGui::Text("请先连接设备并开始捕获");
    }
    
    ImGui::EndChild();
}

void SignaturePanel::RenderControls() {
    ImGui::Text("主要操作:");
    // 设备连接控制
    if (IsDeviceConnected()) {
        if (ImGui::Button("断开设备", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
            DisconnectDevice();
        }
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        if (m_signaturePad && m_signaturePad->IsCapturing()) {
            if (ImGui::Button("停止捕获", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
                StopCapture();
            }
        } else {
            if (ImGui::Button("开始捕获", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
                StartCapture();
            }
        }
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        if (ImGui::Button("清除轨迹", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
            ClearSignature();
        }
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        if (ImGui::Button("获取数据", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
            if (m_signaturePad) {
                SignatureData data;
                if (m_signaturePad->GetSignatureData(data)) {
                    m_currentSignature = data;
                    m_hasSignature = true;
                    LOG_INFO("Signature data retrieved successfully");
                } else {
                    LOG_WARNING("No signature data available");
                }
            }
        }
    } else {
        if (ImGui::Button("连接设备", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
            ConnectDevice();
        }
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        ImGui::BeginDisabled();
        ImGui::Button("开始捕获", ImVec2(kPanelButtonWidth, kPanelButtonHeight));
        ImGui::EndDisabled();
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        ImGui::BeginDisabled();
        ImGui::Button("清除轨迹", ImVec2(kPanelButtonWidth, kPanelButtonHeight));
        ImGui::EndDisabled();
        
        ContinueOnSameLineIfFits(kPanelButtonWidth);
        ImGui::BeginDisabled();
        ImGui::Button("获取数据", ImVec2(kPanelButtonWidth, kPanelButtonHeight));
        ImGui::EndDisabled();
    }
    
    ImGui::Separator();
    ImGui::Text("显示控制:");
    
    // 全屏模式控制
    if (ImGui::Button("全屏模式", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
        if (m_signaturePad) {
            m_fullscreen = !m_fullscreen;
            m_signaturePad->SetFullscreen(m_fullscreen);
            LOG_INFO("Fullscreen mode: " + std::string(m_fullscreen ? "ON" : "OFF"));
        }
    }
    
    ImGui::Separator();
    ImGui::Text("库管理:");
    if (ImGui::Button("库设置", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
        m_showLibrarySettings = !m_showLibrarySettings;
    }
    ContinueOnSameLineIfFits(kPanelButtonWidth);
    if (ImGui::Button("重新加载库", ImVec2(kPanelButtonWidth, kPanelButtonHeight))) {
        LoadSignatureLibrary();
    }
    
    ImGui::Separator();
}

void SignaturePanel::RenderLibrarySettings() {
    if (ImGui::Begin("手写屏库设置", &m_showLibrarySettings)) {
        ImGui::Text("库文件路径:");
        ImGui::SameLine();
        
        char pathBuffer[512];
        strncpy_s(pathBuffer, m_libraryPath.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        
        if (ImGui::InputText("##LibraryPath", pathBuffer, sizeof(pathBuffer))) {
            m_libraryPath = std::string(pathBuffer);
        }
        
        ImGui::SameLine();
        if (ImGui::Button("浏览...", ImVec2(80, 20))) {
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
        }
        
        ImGui::Separator();
        
        ImGui::Text("默认加载:");
        ImGui::BulletText("系统DLL: CMCC_SIGN.DLL");
        ImGui::BulletText("系统会自动在系统目录中查找");
        ImGui::BulletText("包括: System32, SysWOW64, PATH环境变量等");
        
        ImGui::Separator();
        
        if (ImGui::Button("应用设置", ImVec2(100, 30))) {
            LoadSignatureLibrary();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("关闭", ImVec2(100, 30))) {
            m_showLibrarySettings = false;
        }
        
        ImGui::Separator();
        ImGui::Text("说明:");
        ImGui::Text("- 库文件路径: 手写屏DLL文件路径");
        ImGui::Text("- 支持的库: cmcc_sign.dll 等标准接口库");
        ImGui::Text("- 修改后可点击 [应用设置] 重新记录配置");
    }
    ImGui::End();
}

// 设备控制方法实现
void SignaturePanel::ConnectDevice() {
    LOG_INFO("Connecting signature pad device");
    
    // 从 DeviceManager 获取设备
    m_signaturePad.reset(DeviceManager::Instance().GetSignaturePad());
    
    if (!m_signaturePad) {
        LOG_ERROR("No signature pad device available");
        return;
    }
    
    if (m_signaturePad->Initialize()) {
        LOG_INFO("Signature pad device connected successfully");
    } else {
        LOG_ERROR("Failed to connect signature pad device");
    }
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
    
    // 设备库的加载由设备本身处理
    // 这里主要是记录配置
    LOG_INFO("Library path configured: " + m_libraryPath);
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
    if (IsDeviceConnected()) {
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 绿色
    } else {
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 红色
    }
}

} // namespace AsTestTool
