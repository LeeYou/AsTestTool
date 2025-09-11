#include "gui/CameraPanel.h"
#include "utils/Logger.h"
#include "imgui.h"

namespace AsTestTool {

CameraPanel::CameraPanel() {
    LOG_INFO("CameraPanel created");
}

CameraPanel::~CameraPanel() {
    LOG_INFO("CameraPanel destroyed");
}

void CameraPanel::Render() {
    RenderDeviceStatus();
    RenderControls();
    RenderSettings();
    RenderPreview();
}

void CameraPanel::RenderDeviceStatus() {
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "未连接");
    
    ImGui::Separator();
}

void CameraPanel::RenderPreview() {
    ImGui::Text("摄像头预览:");
    ImGui::Separator();
    
    // 预览区域
    ImVec2 previewSize = ImVec2(320, 240);
    ImGui::BeginChild("Preview", previewSize, true, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("预览区域");
    ImGui::Text("分辨率: 未设置");
    ImGui::EndChild();
}

void CameraPanel::RenderControls() {
    if (ImGui::Button("选择设备", ImVec2(100, 30))) {
        LOG_INFO("选择摄像头设备");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("开始预览", ImVec2(100, 30))) {
        LOG_INFO("开始摄像头预览");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("拍照", ImVec2(100, 30))) {
        LOG_INFO("拍照");
    }
    
    ImGui::Separator();
}

void CameraPanel::RenderSettings() {
    ImGui::Text("摄像头设置:");
    ImGui::Separator();
    
    static int selectedResolution = 0;
    const char* resolutions[] = { "640x480", "1280x720", "1920x1080", "3840x2160" };
    ImGui::Combo("分辨率", &selectedResolution, resolutions, IM_ARRAYSIZE(resolutions));
    
    static int brightness = 50;
    ImGui::SliderInt("亮度", &brightness, 0, 100);
    
    static int contrast = 50;
    ImGui::SliderInt("对比度", &contrast, 0, 100);
    
    ImGui::Separator();
}

} // namespace AsTestTool
