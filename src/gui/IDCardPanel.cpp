#include "gui/IDCardPanel.h"
#include "utils/Logger.h"
#include "imgui.h"

namespace AsTestTool {

IDCardPanel::IDCardPanel() {
    LOG_INFO("IDCardPanel created");
}

IDCardPanel::~IDCardPanel() {
    LOG_INFO("IDCardPanel destroyed");
}

void IDCardPanel::Render() {
    RenderDeviceStatus();
    RenderControls();
    RenderCardInfo();
}

void IDCardPanel::RenderDeviceStatus() {
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "未连接");
    
    ImGui::Separator();
}

void IDCardPanel::RenderCardInfo() {
    ImGui::Text("身份证信息:");
    ImGui::Separator();
    
    ImGui::Text("姓名: 未读取");
    ImGui::Text("性别: 未读取");
    ImGui::Text("民族: 未读取");
    ImGui::Text("出生日期: 未读取");
    ImGui::Text("住址: 未读取");
    ImGui::Text("身份证号: 未读取");
    ImGui::Text("签发机关: 未读取");
    ImGui::Text("有效期限: 未读取");
    
    ImGui::Separator();
    ImGui::Text("照片: 无");
}

void IDCardPanel::RenderControls() {
    if (ImGui::Button("连接设备", ImVec2(100, 30))) {
        LOG_INFO("连接身份证阅读器设备");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("读取身份证", ImVec2(100, 30))) {
        LOG_INFO("读取身份证");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("弹出卡片", ImVec2(100, 30))) {
        LOG_INFO("弹出身份证卡片");
    }
    
    ImGui::Separator();
}

} // namespace AsTestTool
