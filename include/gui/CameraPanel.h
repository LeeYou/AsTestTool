#pragma once

namespace AsTestTool {

/**
 * @brief 摄像头面板类
 */
class CameraPanel {
public:
    CameraPanel();
    ~CameraPanel();

    /**
     * @brief 渲染面板
     */
    void Render();

private:
    void RenderDeviceStatus();
    void RenderPreview();
    void RenderControls();
    void RenderSettings();
};

} // namespace AsTestTool
