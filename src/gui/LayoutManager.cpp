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
    float maxDebugHeight = std::max(0.0f, windowSize.y - m_config.minPanelHeight);
    return std::clamp(debugHeight, 0.0f, maxDebugHeight);
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
        float singleHeight = std::max(1.0f, mainPanel.size.y / 3.0f);
        float yOffset = index * singleHeight;
        float remainingHeight = std::max(0.0f, mainPanel.size.y - yOffset);
        float panelHeight = (index == 2) ? remainingHeight : std::min(singleHeight, remainingHeight);
        
        layout.position = ImVec2(mainPanel.position.x, mainPanel.position.y + yOffset);
        layout.size = ImVec2(mainPanel.size.x, panelHeight);
    } else {
        float panelWidth = GetPanelWidth(mainPanel.size.x, 3);
        float xOffset = index * panelWidth;
        float remainingWidth = std::max(0.0f, mainPanel.size.x - xOffset);
        float actualWidth = (index == 2) ? remainingWidth : std::min(panelWidth, remainingWidth);
        
        layout.position = ImVec2(mainPanel.position.x + xOffset, mainPanel.position.y);
        layout.size = ImVec2(actualWidth, mainPanel.size.y);
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
