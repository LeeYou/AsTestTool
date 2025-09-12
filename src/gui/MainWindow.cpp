#include "gui/MainWindow.h"
#include "utils/Logger.h"
#include "utils/LogDisplay.h"
#include "gui/IDCardPanel.h"
#include "gui/CameraPanel.h"
#include "gui/SignaturePanel.h"
#include "gui/Theme.h"
#include "core/DeviceFactory.h"
#include "plugins/CameraManager.h"

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
    
    // 初始化设备
    InitializeDevices();
    
    // 创建各个面板
    m_idCardPanel = std::make_unique<IDCardPanel>();
    m_cameraPanel = std::make_unique<CameraPanel>();
    m_signaturePanel = std::make_unique<SignaturePanel>();
    
    // 设置设备到面板
    if (m_cameraManager) {
        m_cameraPanel->SetCameraManager(m_cameraManager);
    }
    
    // 应用亮色主题
    Theme::Instance().ApplyLightTheme();
    
    m_initialized = true;
    return true;
}

void MainWindow::InitializeDevices() {
    LOG_INFO("Initializing devices");
    
    // 创建摄像头管理器
    m_cameraManager = std::make_shared<Plugins::CameraManager>();
    if (m_cameraManager && m_cameraManager->Initialize()) {
        LOG_INFO("Camera manager created and initialized successfully");
    } else {
        LOG_WARNING("Failed to create or initialize camera manager");
    }
    
    // 创建身份证阅读器设备
    m_idCardReader = DeviceFactory::CreateIDCardReader("default");
    if (m_idCardReader) {
        LOG_INFO("ID Card Reader device created successfully");
    } else {
        LOG_WARNING("Failed to create ID Card Reader device");
    }
    
    // 创建手写屏设备
    m_signaturePad = DeviceFactory::CreateSignaturePad("default");
    if (m_signaturePad) {
        LOG_INFO("Signature Pad device created successfully");
    } else {
        LOG_WARNING("Failed to create Signature Pad device");
    }
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
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                   ImGuiWindowFlags_NoBackground;
    
    if (ImGui::Begin("MainWindow", nullptr, window_flags)) {
        // 渲染菜单栏
        RenderMenuBar();
        
        // 渲染主内容
        RenderMainContent();
        
        // 渲染状态栏
        RenderStatusBar();
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
    // 获取整个窗口的可用区域（减去菜单栏和状态栏的高度）
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    
    // 设置最小尺寸限制
    const float minWidth = 800.0f;
    const float minHeight = 600.0f;
    const float minPanelWidth = 200.0f;
    const float minDebugHeight = 120.0f;
    
    // 如果窗口太小，使用垂直堆叠布局
    bool useVerticalLayout = (windowSize.x < minWidth || windowSize.y < minHeight);
    
    float panelHeight, debugHeight, panelWidth;
    
    if (useVerticalLayout) {
        // 垂直堆叠布局：设备面板在上，调试面板在下
        debugHeight = std::max(minDebugHeight, windowSize.y * 0.25f);
        panelHeight = windowSize.y - debugHeight;
        panelWidth = windowSize.x; // 每个面板占满宽度
    } else {
        // 水平布局：三个设备面板并排，调试面板在下方
        debugHeight = std::max(minDebugHeight, windowSize.y * 0.2f);
        panelHeight = windowSize.y - debugHeight;
        panelWidth = std::max(minPanelWidth, windowSize.x / 3.0f);
    }
    
    // 设备面板布局
    if (useVerticalLayout) {
        // 垂直堆叠布局：每个面板占满宽度，垂直排列
        float singlePanelHeight = panelHeight / 3.0f;
        
        // 身份证阅读器面板 - 顶部
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, singlePanelHeight), ImGuiCond_Always);
        if (ImGui::Begin("身份证阅读器", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_idCardPanel) {
                m_idCardPanel->Render();
            }
        }
        ImGui::End();
        
        // 摄像头面板 - 中间
        ImGui::SetNextWindowPos(ImVec2(0, singlePanelHeight), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, singlePanelHeight), ImGuiCond_Always);
        if (ImGui::Begin("摄像头/高拍仪", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_cameraPanel) {
                m_cameraPanel->Render();
            }
        }
        ImGui::End();
        
        // 手写屏面板 - 底部
        ImGui::SetNextWindowPos(ImVec2(0, singlePanelHeight * 2), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, singlePanelHeight), ImGuiCond_Always);
        if (ImGui::Begin("手写屏", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_signaturePanel) {
                m_signaturePanel->Render();
            }
        }
        ImGui::End();
    } else {
        // 水平布局：三个设备面板并排排列
        // 身份证阅读器面板 - 左侧
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
        if (ImGui::Begin("身份证阅读器", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_idCardPanel) {
                m_idCardPanel->Render();
            }
        }
        ImGui::End();
        
        // 摄像头面板 - 中间
        ImGui::SetNextWindowPos(ImVec2(panelWidth, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
        if (ImGui::Begin("摄像头/高拍仪", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_cameraPanel) {
                m_cameraPanel->Render();
            }
        }
        ImGui::End();
        
        // 手写屏面板 - 右侧
        ImGui::SetNextWindowPos(ImVec2(panelWidth * 2, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
        if (ImGui::Begin("手写屏", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            if (m_signaturePanel) {
                m_signaturePanel->Render();
            }
        }
        ImGui::End();
    }
    
    // 下方：调试信息窗口，填充满整个宽度
    ImGui::SetNextWindowPos(ImVec2(0, panelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, debugHeight), ImGuiCond_Always);
    if (ImGui::Begin("调试信息", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        RenderDebugInfo();
    }
    ImGui::End();
}

void MainWindow::RenderStatusBar() {
    // 简化的状态栏
    ImGui::Separator();
    ImGui::Text("就绪 | 设备状态: 未连接 | 版本: 1.0.0 | AsTestTool v1.0");
}

MainWindow::DeviceStatus MainWindow::GetDeviceStatus() const {
    DeviceStatus status;
    
    // 获取摄像头状态
    if (m_cameraManager) {
        status.cameraConnected = m_cameraManager->IsCameraOpen();
    }
    
    // 获取身份证阅读器状态
    if (m_idCardReader) {
        status.idCardConnected = m_idCardReader->IsConnected();
    }
    
    // 获取手写屏状态
    if (m_signaturePad) {
        status.signatureConnected = m_signaturePad->IsConnected();
    }
    
    return status;
}

void MainWindow::RenderDebugInfo() {
    // 调试信息显示
    ImGui::Text("调试信息:");
    ImGui::Separator();
    
    // 获取统一的设备状态
    DeviceStatus status = GetDeviceStatus();
    
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
