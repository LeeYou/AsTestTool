#include "gui/SignaturePanel.h"
#include "utils/Logger.h"
#include "utils/FileDialog.h"
#include "core/DeviceManager.h"
#include "imgui.h"

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
    
    // 手写区域
    ImVec2 signatureSize = ImVec2(400, 200);
    ImGui::BeginChild("Signature", signatureSize, true, ImGuiWindowFlags_NoScrollbar);
    
    if (m_hasSignature && m_currentSignature.HasData()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== 手写数据已捕获 ===");
        ImGui::Text("轨迹点数: %zu", m_currentSignature.GetPointCount());
        ImGui::Text("屏幕尺寸: %dx%d", m_currentSignature.width, m_currentSignature.height);
        ImGui::Text("设备信息: %s", m_currentSignature.deviceInfo.c_str());
        
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
    // 设备连接控制
    if (IsDeviceConnected()) {
        if (ImGui::Button("断开设备", ImVec2(100, 30))) {
            DisconnectDevice();
        }
        
        ImGui::SameLine();
        if (m_signaturePad && m_signaturePad->IsCapturing()) {
            if (ImGui::Button("停止捕获", ImVec2(100, 30))) {
                StopCapture();
            }
        } else {
            if (ImGui::Button("开始捕获", ImVec2(100, 30))) {
                StartCapture();
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("清除轨迹", ImVec2(100, 30))) {
            ClearSignature();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("获取数据", ImVec2(100, 30))) {
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
        if (ImGui::Button("连接设备", ImVec2(100, 30))) {
            ConnectDevice();
        }
        
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Button("开始捕获", ImVec2(100, 30));
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Button("清除轨迹", ImVec2(100, 30));
        ImGui::EndDisabled();
        
        ImGui::SameLine();
        ImGui::BeginDisabled();
        ImGui::Button("获取数据", ImVec2(100, 30));
        ImGui::EndDisabled();
    }
    
    ImGui::Separator();
    
    // 全屏模式控制
    if (ImGui::Button("全屏模式", ImVec2(100, 30))) {
        if (m_signaturePad) {
            m_fullscreen = !m_fullscreen;
            m_signaturePad->SetFullscreen(m_fullscreen);
            LOG_INFO("Fullscreen mode: " + std::string(m_fullscreen ? "ON" : "OFF"));
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("库设置", ImVec2(100, 30))) {
        m_showLibrarySettings = !m_showLibrarySettings;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("加载库", ImVec2(100, 30))) {
        LoadSignatureLibrary();
    }
    
    ImGui::Separator();
}

void SignaturePanel::RenderLibrarySettings() {
    ImGui::Separator();
    ImGui::Text("库设置:");
    
    char pathBuffer[512];
    strncpy_s(pathBuffer, m_libraryPath.c_str(), sizeof(pathBuffer) - 1);
    pathBuffer[sizeof(pathBuffer) - 1] = '\0';
    
    if (ImGui::InputText("DLL路径", pathBuffer, sizeof(pathBuffer))) {
        m_libraryPath = std::string(pathBuffer);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("浏览...", ImVec2(80, 20))) {
        // 使用跨平台文件对话框
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
    
    ImGui::Text("当前库: %s", m_libraryPath.c_str());
    
    // 显示默认路径信息
    ImGui::Separator();
    ImGui::Text("默认加载:");
    ImGui::BulletText("系统DLL: CMCC_SIGN.DLL");
    ImGui::BulletText("系统会自动在系统目录中查找");
    ImGui::BulletText("包括: System32, SysWOW64, PATH环境变量等");
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
