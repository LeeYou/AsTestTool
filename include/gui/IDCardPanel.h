#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include <memory>
#include <string>

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
 * @brief 身份证面板类
 */
class IDCardPanel {
public:
    IDCardPanel();
    ~IDCardPanel();

    /**
     * @brief 设置身份证阅读器设备
     * @param reader 身份证阅读器设备
     */
    void SetIDCardReader(IIDCardReader* reader);

    /**
     * @brief 渲染面板
     */
    void Render();

private:
    void RenderDeviceStatus();
    void RenderCardInfo();
    void RenderControls();
    void RenderLibrarySettings();
    
    // 设备操作
    void ConnectDevice();
    void DisconnectDevice();
    void ReadCard();
    void EjectCard();
    void LoadIDCardLibrary();
    
    // 状态管理
    bool IsDeviceConnected() const;
    std::string GetDeviceStatusText() const;
    ImVec4 GetDeviceStatusColor() const;

private:
    IIDCardReader* m_idCardReader = nullptr;
    std::unique_ptr<IIDCardReader> m_customReader;
    IDCardInfo m_currentCardInfo;
    bool m_hasCardInfo = false;
    
    // 库设置
    std::string m_libraryPath = "cmcc_idcard.dll";
    int m_devicePort = 1001;
    bool m_showLibrarySettings = false;
};

} // namespace AsTestTool
