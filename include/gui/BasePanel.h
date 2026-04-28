#pragma once

#include <string>
#include <memory>
#include <functional>

// ImGui 头文件
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <imgui.h>

namespace AsTestTool {

/**
 * @brief 面板基类
 * 
 * 所有设备面板的基类，提供通用功能：
 * - 设备状态管理
 * - 渲染布局支持
 * - 状态显示
 */
class BasePanel {
public:
    /**
     * @brief 连接状态
     */
    enum class ConnectionStatus {
        Disconnected,   // 未连接
        Connecting,      // 连接中
        Connected,       // 已连接
        Error           // 连接错误
    };

    /**
     * @brief 获取面板名称
     */
    virtual const char* GetPanelName() const = 0;

    /**
     * @brief 渲染面板
     */
    virtual void Render() = 0;

    /**
     * @brief 处理键盘快捷键
     */
    virtual void HandleKeyboardShortcuts() {}

    /**
     * @brief 获取连接状态
     */
    ConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }

    /**
     * @brief 获取状态文本
     */
    virtual std::string GetStatusText() const;

    /**
     * @brief 获取设备信息文本
     */
    virtual std::string GetDeviceInfoText() const;

    /**
     * @brief 获取状态颜色
     */
    virtual ImVec4 GetStatusColor() const;

    /**
     * @brief 检查是否已连接
     */
    bool IsConnected() const { return m_connectionStatus == ConnectionStatus::Connected; }

protected:
    /**
     * @brief 渲染设备状态区域
     */
    void RenderDeviceStatus();

    /**
     * @brief 渲染通用控制按钮
     */
    void RenderCommonControls(
        std::function<bool()> connectCallback,
        std::function<bool()> disconnectCallback,
        std::function<bool()> refreshCallback
    );

    /**
     * @brief 设置连接状态
     */
    void SetConnectionStatus(ConnectionStatus status) { m_connectionStatus = status; }

protected:
    ConnectionStatus m_connectionStatus = ConnectionStatus::Disconnected;
};

} // namespace AsTestTool

// 状态颜色辅助函数声明
ImVec4 GetColorForStatus(AsTestTool::BasePanel::ConnectionStatus status);
