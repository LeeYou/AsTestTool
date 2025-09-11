#include "gui/SignaturePanel.h"
#include "utils/Logger.h"
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
}

void SignaturePanel::RenderDeviceStatus() {
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "未连接");
    
    ImGui::Separator();
}

void SignaturePanel::RenderSignature() {
    ImGui::Text("手写轨迹:");
    ImGui::Separator();
    
    // 手写区域
    ImVec2 signatureSize = ImVec2(400, 200);
    ImGui::BeginChild("Signature", signatureSize, true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("手写区域");
    ImGui::Text("轨迹点数: 0");
    ImGui::Text("屏幕尺寸: 未获取");
    ImGui::EndChild();
}

void SignaturePanel::RenderControls() {
    if (ImGui::Button("连接设备", ImVec2(100, 30))) {
        LOG_INFO("连接手写屏设备");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("开始测试", ImVec2(100, 30))) {
        LOG_INFO("开始手写屏测试");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("清除轨迹", ImVec2(100, 30))) {
        LOG_INFO("清除手写轨迹");
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("全屏模式", ImVec2(100, 30))) {
        LOG_INFO("切换到全屏模式");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("获取数据", ImVec2(100, 30))) {
        LOG_INFO("获取手写数据");
    }
    
    ImGui::Separator();
}

} // namespace AsTestTool
