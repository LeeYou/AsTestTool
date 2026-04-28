#include "gui/LayoutManager.h"
#include <algorithm>
#include <cmath>

namespace AsTestTool {

LayoutManager::LayoutManager() = default;

void LayoutManager::SetConfig(const LayoutConfig& config) {
    m_config = config;
}

LayoutManager::LayoutMode LayoutManager::CalculateMode(const ImVec2& windowSize) {
    if (windowSize.x < m_config.compactBreakpoint || windowSize.y < m_config.minWindowHeight) {
        m_currentMode = LayoutMode::Compact;
    } else if (windowSize.x > m_config.wideBreakpoint) {
        m_currentMode = LayoutMode::Wide;
    } else {
        m_currentMode = LayoutMode::Standard;
    }
    return m_currentMode;
}

float LayoutManager::CalculateDebugHeight(const ImVec2& windowSize) {
    float debugHeight = windowSize.y * m_config.debugPanelRatio;
    return std::max(m_config.minDebugHeight, debugHeight);
}

bool LayoutManager::ShouldUseVerticalLayout(const ImVec2& windowSize) {
    return windowSize.x < m_config.minWindowWidth || windowSize.y < m_config.minWindowHeight;
}

float LayoutManager::GetPanelWidth(float totalWidth, int panelCount) {
    float width = totalWidth / panelCount;
    return std::max(m_config.minPanelWidth, width);
}

LayoutManager::PanelLayout LayoutManager::CalculateMainPanelLayout(const ImVec2& windowSize, float debugHeight) {
    PanelLayout layout;
    layout.position = ImVec2(0, 0);
    layout.size = ImVec2(windowSize.x, windowSize.y - debugHeight);
    layout.contentSize = layout.size;
    return layout;
}

LayoutManager::PanelLayout LayoutManager::CalculateDebugPanelLayout(const ImVec2& windowSize, float debugHeight) {
    PanelLayout layout;
    layout.position = ImVec2(0, windowSize.y - debugHeight);
    layout.size = ImVec2(windowSize.x, debugHeight);
    layout.contentSize = layout.size;
    return layout;
}

LayoutManager::PanelLayout LayoutManager::CalculateDevicePanelLayout(int index, const PanelLayout& mainPanel, bool useVertical) {
    PanelLayout layout;
    
    if (useVertical) {
        // 垂直布局：每个面板占满宽度，垂直排列
        float singleHeight = mainPanel.size.y / 3.0f;
        singleHeight = std::max(m_config.minPanelHeight, singleHeight);
        
        layout.position = ImVec2(0, mainPanel.position.y + index * singleHeight);
        layout.size = ImVec2(mainPanel.size.x, singleHeight);
    } else {
        // 水平布局：三个面板并排
        float panelWidth = GetPanelWidth(mainPanel.size.x, 3);
        
        layout.position = ImVec2(mainPanel.position.x + index * panelWidth, mainPanel.position.y);
        layout.size = ImVec2(panelWidth, mainPanel.size.y);
    }
    
    layout.contentSize = layout.size;
    return layout;
}

ImVec2 LayoutManager::CalculatePreviewSize(const ImVec2& availableSize, float aspectRatio) {
    float width = availableSize.x;
    float height = width / aspectRatio;
    
    if (height > availableSize.y) {
        height = availableSize.y;
        width = height * aspectRatio;
    }
    
    // 确保不超过可用区域
    width = std::min(width, availableSize.x);
    height = std::min(height, availableSize.y);
    
    return ImVec2(width, height);
}

ImVec2 LayoutManager::CalculatePreviewPosition(const ImVec2& previewSize, const ImVec2& availableSize) {
    ImVec2 pos;
    pos.x = (availableSize.x - previewSize.x) * 0.5f;
    pos.y = (availableSize.y - previewSize.y) * 0.5f;
    return pos;
}

} // namespace AsTestTool
