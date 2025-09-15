#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include <memory>
#include <string>

// Forward declaration for ImGui types
struct ImVec4;

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
    void SetIDCardReader(std::shared_ptr<IIDCardReader> reader);

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
    std::shared_ptr<IIDCardReader> m_idCardReader;
    IDCardInfo m_currentCardInfo;
    bool m_hasCardInfo = false;
    
    // 库设置
    std::string m_libraryPath = "cmcc_idcard.dll";
    int m_devicePort = 1001;
    bool m_showLibrarySettings = false;
};

} // namespace AsTestTool
