#pragma once

#include <string>

// ImGui 头文件（必须在 Windows GL 头文件之前）
#ifdef _WIN32
// Windows 需要先包含 windows.h 才能安全地包含 GL 头文件
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// 包含 ImGui 的 ImVec2 定义
#include <imgui.h>

namespace AsTestTool {

/**
 * @brief 布局管理器类
 * 
 * 统一管理界面布局计算，解决硬编码布局问题。
 * 支持：
 * - 响应式布局模式（紧凑/标准/宽屏）
 * - 自适应面板尺寸计算
 * - 预览区域尺寸计算
 */
class LayoutManager {
public:
    /**
     * @brief 布局模式
     */
    enum class LayoutMode {
        Compact,    // 紧凑布局（窄窗口）
        Standard,   // 标准布局（中等窗口）
        Wide        // 宽屏布局（大窗口）
    };

    /**
     * @brief 布局配置
     */
    struct LayoutConfig {
        // 最小尺寸限制
        float minWindowWidth = 800.0f;
        float minWindowHeight = 600.0f;
        float minPanelWidth = 200.0f;
        float minPanelHeight = 100.0f;
        float minDebugHeight = 120.0f;

        // 布局比例
        float debugPanelRatio = 0.2f;       // 调试面板占窗口高度比例
        float controlPanelRatio = 0.3f;     // 控制区域占面板高度比例
        float imageInfoRatio = 0.25f;       // 图像信息区域占面板高度比例

        // 断点阈值
        float compactBreakpoint = 600.0f;   // 紧凑模式断点
        float wideBreakpoint = 1000.0f;      // 宽屏模式断点
    };

    /**
     * @brief 面板布局信息
     */
    struct PanelLayout {
        ImVec2 position;    // 左上角位置
        ImVec2 size;        // 尺寸
        ImVec2 contentSize;  // 内容区域尺寸
    };

public:
    LayoutManager();
    
    /**
     * @brief 设置布局配置
     */
    void SetConfig(const LayoutConfig& config);

    /**
     * @brief 获取当前配置
     */
    const LayoutConfig& GetConfig() const { return m_config; }

    /**
     * @brief 根据窗口大小计算布局模式
     * @param windowSize 窗口大小
     * @return 布局模式
     */
    LayoutMode CalculateMode(const ImVec2& windowSize);

    /**
     * @brief 获取当前布局模式
     */
    LayoutMode GetCurrentMode() const { return m_currentMode; }

    /**
     * @brief 计算主面板区域布局（三个设备面板）
     * @param windowSize 窗口大小
     * @param debugHeight 调试面板高度
     * @return 主面板区域布局
     */
    PanelLayout CalculateMainPanelLayout(const ImVec2& windowSize, float debugHeight);

    /**
     * @brief 计算调试面板布局
     * @param windowSize 窗口大小
     * @param debugHeight 调试面板高度
     * @return 调试面板布局
     */
    PanelLayout CalculateDebugPanelLayout(const ImVec2& windowSize, float debugHeight);

    /**
     * @brief 计算单个设备面板的布局
     * @param index 面板索引 (0-2)
     * @param mainPanel 主面板布局
     * @param useVertical 是否使用垂直布局
     * @return 设备面板布局
     */
    PanelLayout CalculateDevicePanelLayout(int index, const PanelLayout& mainPanel, bool useVertical);

    /**
     * @brief 计算预览区域尺寸
     * @param availableSize 可用区域大小
     * @param aspectRatio 期望的宽高比
     * @return 预览区域尺寸
     */
    ImVec2 CalculatePreviewSize(const ImVec2& availableSize, float aspectRatio);

    /**
     * @brief 计算预览区域居中位置
     * @param previewSize 预览尺寸
     * @param availableSize 可用区域大小
     * @return 预览区域左上角位置
     */
    ImVec2 CalculatePreviewPosition(const ImVec2& previewSize, const ImVec2& availableSize);

    /**
     * @brief 计算调试面板高度
     * @param windowSize 窗口大小
     * @return 调试面板高度
     */
    float CalculateDebugHeight(const ImVec2& windowSize);

    /**
     * @brief 检查是否使用垂直布局
     * @param windowSize 窗口大小
     * @return true 使用垂直布局，false 使用水平布局
     */
    bool ShouldUseVerticalLayout(const ImVec2& windowSize);

    /**
     * @brief 获取面板宽度
     * @param totalWidth 总宽度
     * @param panelCount 面板数量
     * @return 单个面板宽度
     */
    float GetPanelWidth(float totalWidth, int panelCount = 3);

private:
    LayoutConfig m_config;
    LayoutMode m_currentMode = LayoutMode::Standard;
};

} // namespace AsTestTool
