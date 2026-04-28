#include "gui/MainWindow.h"
#include "utils/Logger.h"
#include "utils/LogDisplay.h"
#include "gui/IDCardPanel.h"
#include "gui/CameraPanel.h"
#include "gui/SignaturePanel.h"
#include "gui/Theme.h"
#include "gui/LayoutManager.h"
#include "core/DeviceManager.h"
#include "plugins/CameraManager.h"
#include <algorithm>

// ImGui includes
#include "imgui.h"

namespace AsTestTool {

MainWindow::MainWindow() {
    LOG_INFO("MainWindow created");
}

MainWindow::~MainWindow() {
    LOG_INFO("MainWindow destroyed");
}

bool MainWindow::Initialize() {
    LOG_INFO("Initializing MainWindow");
    
    // 初始化面板
    InitializePanels();
    
    // 应用亮色主题
    Theme::Instance().ApplyLightTheme();
    
    m_initialized = true;
    return true;
}

void MainWindow::InitializePanels() {
    LOG_INFO("Initializing panels");
    
    // 创建摄像头管理器
    m_cameraManager = std::make_shared<Plugins::CameraManager>();
    if (m_cameraManager && m_cameraManager->Initialize()) {
        LOG_INFO("Camera manager created and initialized successfully");
    } else {
        LOG_WARNING("Failed to create or initialize camera manager");
    }
    
    // 创建各个面板
    m_idCardPanel = std::make_unique<IDCardPanel>();
    m_cameraPanel = std::make_unique<CameraPanel>();
    m_signaturePanel = std::make_unique<SignaturePanel>();
    
    // 设置设备到面板
    m_cameraPanel->SetCameraManager(m_cameraManager);
    
    LOG_INFO("Panels initialized successfully");
}

void MainWindow::Show() {
    LOG_INFO("Showing MainWindow");
    // TODO: 显示窗口
}

void MainWindow::Hide() {
    LOG_INFO("Hiding MainWindow");
    // TODO: 隐藏窗口
}

void MainWindow::Update() {
    // TODO: 更新窗口状态
}

void MainWindow::Render() {
    if (!m_initialized) return;
    
    // 使用无边框窗口，让内容填充满整个窗口
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                   ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
    
    bool isOpen = ImGui::Begin("MainWindow", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    if (isOpen) {
        // 渲染菜单栏
        RenderMenuBar();
        
        const ImGuiStyle& style = ImGui::GetStyle();
        const float statusBarHeight = ImGui::GetTextLineHeightWithSpacing() + style.FramePadding.y * 2.0f + style.ItemSpacing.y * 2.0f;
        
        ImGui::BeginChild("MainWorkspace", ImVec2(0.0f, -statusBarHeight), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        
        // 渲染主内容
        RenderMainContent();
        ImGui::EndChild();
        
        // 渲染状态栏
        ImGui::BeginChild("StatusBarRegion", ImVec2(0.0f, statusBarHeight), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        RenderStatusBar();
        ImGui::EndChild();
    }
    ImGui::End();
}

bool MainWindow::ShouldClose() const {
    // TODO: 检查窗口关闭条件
    return false;
}

void MainWindow::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("设备")) {
            if (ImGui::MenuItem("身份证阅读器", "F1")) {
                LOG_INFO("身份证阅读器菜单被点击");
            }
            if (ImGui::MenuItem("摄像头", "F2")) {
                LOG_INFO("摄像头菜单被点击");
            }
            if (ImGui::MenuItem("手写屏", "F3")) {
                LOG_INFO("手写屏菜单被点击");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4")) {
                LOG_INFO("退出菜单被点击");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("设置")) {
            if (ImGui::MenuItem("配置", "Ctrl+,,")) {
                LOG_INFO("配置菜单被点击");
            }
            if (ImGui::MenuItem("主题", nullptr)) {
                LOG_INFO("主题菜单被点击");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("帮助")) {
            if (ImGui::MenuItem("关于", "F1")) {
                LOG_INFO("关于菜单被点击");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
}

void MainWindow::RenderMainContent() {
    ImVec2 contentOrigin = ImGui::GetCursorScreenPos();
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    
    // 使用布局管理器计算布局
    m_layoutManager.CalculateMode(windowSize);
    bool useVerticalLayout = m_layoutManager.ShouldUseVerticalLayout(windowSize);
    float debugHeight = m_layoutManager.CalculateDebugHeight(windowSize);
    
    // 计算主面板布局
    auto mainPanelLayout = m_layoutManager.CalculateMainPanelLayout(windowSize, debugHeight);
    auto debugPanelLayout = m_layoutManager.CalculateDebugPanelLayout(windowSize, debugHeight);
    
    // 设备面板数量
    const int panelCount = 3;
    
    // 渲染设备面板
    if (useVerticalLayout) {
        for (int i = 0; i < panelCount; ++i) {
            auto panelLayout = m_layoutManager.CalculateDevicePanelLayout(i, mainPanelLayout, true);
            const char* panelNames[] = {"身份证阅读器", "摄像头/高拍仪", "手写屏"};
            
            ImGui::SetNextWindowPos(ImVec2(contentOrigin.x + panelLayout.position.x, contentOrigin.y + panelLayout.position.y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(panelLayout.size, ImGuiCond_Always);
            
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            
            if (ImGui::Begin(panelNames[i], nullptr, flags)) {
                switch (i) {
                    case 0:
                        if (m_idCardPanel) m_idCardPanel->Render();
                        break;
                    case 1:
                        if (m_cameraPanel) m_cameraPanel->Render();
                        break;
                    case 2:
                        if (m_signaturePanel) m_signaturePanel->Render();
                        break;
                }
            }
            ImGui::End();
        }
    } else {
        for (int i = 0; i < panelCount; ++i) {
            auto panelLayout = m_layoutManager.CalculateDevicePanelLayout(i, mainPanelLayout, false);
            const char* panelNames[] = {"身份证阅读器", "摄像头/高拍仪", "手写屏"};
            
            ImGui::SetNextWindowPos(ImVec2(contentOrigin.x + panelLayout.position.x, contentOrigin.y + panelLayout.position.y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(panelLayout.size, ImGuiCond_Always);
            
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            
            if (ImGui::Begin(panelNames[i], nullptr, flags)) {
                switch (i) {
                    case 0:
                        if (m_idCardPanel) m_idCardPanel->Render();
                        break;
                    case 1:
                        if (m_cameraPanel) m_cameraPanel->Render();
                        break;
                    case 2:
                        if (m_signaturePanel) m_signaturePanel->Render();
                        break;
                }
            }
            ImGui::End();
        }
    }
    
    // 渲染调试信息面板
    ImGui::SetNextWindowPos(ImVec2(contentOrigin.x + debugPanelLayout.position.x, contentOrigin.y + debugPanelLayout.position.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(debugPanelLayout.size, ImGuiCond_Always);
    if (ImGui::Begin("调试信息", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        RenderDebugInfo();
    }
    ImGui::End();
}

void MainWindow::RenderStatusBar() {
    // 获取设备状态
    auto status = DeviceManager::Instance().GetStatus();
    
    // 简化的状态栏
    ImGui::Separator();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("就绪 | ID卡: %s | 摄像头: %s | 手写屏: %s | 版本: 1.0.0",
        status.idCardConnected ? "已连接" : "未连接",
        status.cameraConnected ? "已连接" : "未连接",
        status.signatureConnected ? "已连接" : "未连接");
}

void MainWindow::RenderDebugInfo() {
    // 调试信息显示
    ImGui::Text("调试信息:");
    ImGui::Separator();
    
    // 获取统一的设备状态
    auto status = DeviceManager::Instance().GetStatus();
    
    // 设备状态
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    
    // 身份证阅读器状态
    ImGui::TextColored(status.idCardConnected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                      status.idCardConnected ? "身份证阅读器: 已连接" : "身份证阅读器: 未连接");
    ImGui::SameLine();
    
    // 摄像头状态
    ImGui::TextColored(status.cameraConnected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                      status.cameraConnected ? "摄像头: 已连接" : "摄像头: 未连接");
    ImGui::SameLine();
    
    // 手写屏状态
    ImGui::TextColored(status.signatureConnected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                      status.signatureConnected ? "手写屏: 已连接" : "手写屏: 未连接");
    
    ImGui::Separator();
    
    // 系统信息
    ImGui::Text("系统信息:");
    ImGui::SameLine();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::SameLine();
    ImGui::Text("窗口大小: %.0fx%.0f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    ImGui::SameLine();
    ImGui::Text("版本: AsTestTool v1.0");
    
    ImGui::Separator();
    
    // 实时日志显示 - 支持高度调整
    ImGui::Text("实时日志:");
    ImGui::SameLine();
    
    // 添加日志面板高度控制
    static float logPanelHeight = 100.0f;
    ImGui::SliderFloat("##logHeight", &logPanelHeight, 50.0f, 300.0f, "%.0fpx");
    
    ImGui::BeginChild("LogDisplay", ImVec2(-1, logPanelHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    // 获取日志条目
    const auto& logs = LogDisplay::Instance().GetLogs();
    
    // 显示日志条目
    for (const auto& log : logs) {
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        switch (log.level) {
            case LogDisplayLevel::Error:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case LogDisplayLevel::Warning:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LogDisplayLevel::Info:
                color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LogDisplayLevel::Debug:
                color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                break;
        }
        
        ImGui::TextColored(color, "[%s] %s", log.timestamp.c_str(), log.message.c_str());
    }
    
    // 自动滚动到底部
    static bool autoScroll = true;
    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
        ImGui::SetScrollHereY(1.0f);
    }
    
    // 检查是否用户手动滚动了
    if (ImGui::GetScrollY() < ImGui::GetScrollMaxY() - 1.0f) {
        autoScroll = false;
    }
    
    // 如果有新日志，重新启用自动滚动
    static size_t lastLogCount = 0;
    size_t currentLogCount = LogDisplay::Instance().GetLogCount();
    if (currentLogCount > lastLogCount) {
        autoScroll = true;
        lastLogCount = currentLogCount;
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    
    // 操作提示
    ImGui::Text("操作提示:");
    ImGui::SameLine();
    ImGui::Text("空格键: 拍照 | F11: 全屏 | ESC: 退出全屏");
}

} // namespace AsTestTool
