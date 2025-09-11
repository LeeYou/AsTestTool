#pragma once

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

private:
    void RenderDeviceStatus();
    void RenderSignature();
    void RenderControls();
};

} // namespace AsTestTool
