#pragma once

#include <memory>
#include <vector>
#include "gui/IDCardPanel.h"
#include "gui/CameraPanel.h"
#include "gui/SignaturePanel.h"
#include "gui/LayoutManager.h"
#include "plugins/CameraManager.h"

namespace AsTestTool {

/**
 * @brief 主窗口类
 */
class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    /**
     * @brief 初始化主窗口
     * @return true 初始化成功，false 初始化失败
     */
    bool Initialize();

    /**
     * @brief 显示主窗口
     */
    void Show();

    /**
     * @brief 隐藏主窗口
     */
    void Hide();

    /**
     * @brief 更新窗口
     */
    void Update();

    /**
     * @brief 渲染窗口
     */
    void Render();

    /**
     * @brief 检查窗口是否应该关闭
     * @return true 应该关闭，false 继续运行
     */
    bool ShouldClose() const;

private:
    void RenderMenuBar();
    void RenderMainContent();
    void RenderStatusBar();
    void RenderDebugInfo();
    void InitializePanels();
    
    // 布局管理器
    LayoutManager m_layoutManager;
    
    bool m_initialized = false;
    std::unique_ptr<IDCardPanel> m_idCardPanel;
    std::unique_ptr<CameraPanel> m_cameraPanel;
    std::unique_ptr<SignaturePanel> m_signaturePanel;
    
    // 摄像头管理器（由 CameraPanel 使用）
    std::shared_ptr<Plugins::CameraManager> m_cameraManager;
};

} // namespace AsTestTool
