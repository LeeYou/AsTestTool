#pragma once

#include "devices/interfaces/ISignaturePad.h"
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
 * @brief 手写屏面板类
 */
class SignaturePanel {
public:
    SignaturePanel();
    ~SignaturePanel();

    /**
     * @brief 渲染面板
     */
    void Render();

    /**
     * @brief 设置手写屏设备
     * @param signaturePad 手写屏设备指针
     */
    void SetSignaturePad(ISignaturePad* signaturePad);

private:
    void RenderDeviceStatus();
    void RenderSignature();
    void RenderControls();
    void RenderLibrarySettings();
    
    // 设备控制方法
    void ConnectDevice();
    void DisconnectDevice();
    void StartCapture();
    void StopCapture();
    void ClearSignature();
    void LoadSignatureLibrary();
    
    // 状态检查方法
    bool IsDeviceConnected() const;
    std::string GetDeviceStatusText() const;
    ImVec4 GetDeviceStatusColor() const;

private:
    ISignaturePad* m_signaturePad = nullptr;
    std::unique_ptr<ISignaturePad> m_customSignaturePad;
    
    // 手写数据
    SignatureData m_currentSignature;
    bool m_hasSignature = false;
    
    // 库设置
    std::string m_libraryPath = "cmcc_sign.dll";
    bool m_showLibrarySettings = false;
    
    // 全屏模式
    bool m_fullscreen = false;
};

} // namespace AsTestTool
