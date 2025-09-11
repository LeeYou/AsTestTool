#pragma once

#include <string>
#include <vector>
#include "devices/interfaces/ISignaturePad.h"

namespace AsTestTool {

/**
 * @brief 全屏窗口类
 */
class FullscreenWindow {
public:
    FullscreenWindow();
    ~FullscreenWindow();

    /**
     * @brief 创建全屏窗口
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return true 创建成功，false 创建失败
     */
    bool Create(int width, int height);

    /**
     * @brief 显示窗口
     */
    void Show();

    /**
     * @brief 隐藏窗口
     */
    void Hide();

    /**
     * @brief 绘制手写轨迹
     * @param points 手写点数据
     */
    void DrawSignature(const std::vector<SignaturePoint>& points);

    /**
     * @brief 检查窗口是否可见
     * @return true 可见，false 不可见
     */
    bool IsVisible() const;

    /**
     * @brief 更新窗口
     */
    void Update();

    /**
     * @brief 渲染窗口
     */
    void Render();

    /**
     * @brief 关闭窗口
     */
    void Close();

private:
    bool m_created = false;
    bool m_visible = false;
    int m_width = 0;
    int m_height = 0;
};

} // namespace AsTestTool
