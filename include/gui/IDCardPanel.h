#pragma once

namespace AsTestTool {

/**
 * @brief 身份证面板类
 */
class IDCardPanel {
public:
    IDCardPanel();
    ~IDCardPanel();

    /**
     * @brief 渲染面板
     */
    void Render();

private:
    void RenderDeviceStatus();
    void RenderCardInfo();
    void RenderControls();
};

} // namespace AsTestTool
