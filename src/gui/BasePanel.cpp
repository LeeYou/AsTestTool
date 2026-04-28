#include "gui/BasePanel.h"
#include <functional>

namespace AsTestTool {

std::string BasePanel::GetStatusText() const {
    switch (m_connectionStatus) {
        case ConnectionStatus::Disconnected:
            return "未连接";
        case ConnectionStatus::Connecting:
            return "连接中...";
        case ConnectionStatus::Connected:
            return "已连接";
        case ConnectionStatus::Error:
            return "连接错误";
        default:
            return "未知状态";
    }
}

ImVec4 BasePanel::GetStatusColor() const {
    return GetColorForStatus(m_connectionStatus);
}

void BasePanel::RenderDeviceStatus() {
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    ImVec4 statusColor = GetStatusColor();
    ImGui::TextColored(statusColor, "%s", GetStatusText().c_str());
    
    std::string deviceInfo = GetDeviceInfoText();
    if (!deviceInfo.empty()) {
        ImGui::Text("设备信息: %s", deviceInfo.c_str());
    }
    
    ImGui::Separator();
}

void BasePanel::RenderCommonControls(
    std::function<bool()> connectCallback,
    std::function<bool()> disconnectCallback,
    std::function<bool()> refreshCallback
) {
    if (IsConnected()) {
        if (ImGui::Button("断开连接", ImVec2(100, 30))) {
            disconnectCallback();
        }
    } else {
        if (ImGui::Button("连接设备", ImVec2(100, 30))) {
            connectCallback();
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("刷新设备", ImVec2(100, 30))) {
        refreshCallback();
    }
    
    ImGui::Separator();
}

std::string BasePanel::GetDeviceInfoText() const {
    return "";  // 默认返回空字符串，子类可以重写此方法
}

} // namespace AsTestTool

// 命名空间外的自由函数实现
ImVec4 GetColorForStatus(AsTestTool::BasePanel::ConnectionStatus status) {
    switch (status) {
        case AsTestTool::BasePanel::ConnectionStatus::Connected:
            return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // 绿色
        case AsTestTool::BasePanel::ConnectionStatus::Connecting:
            return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色
        case AsTestTool::BasePanel::ConnectionStatus::Error:
            return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
        case AsTestTool::BasePanel::ConnectionStatus::Disconnected:
        default:
            return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // 灰色
    }
}
