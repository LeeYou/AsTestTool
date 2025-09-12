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
    
    // 计算布局尺寸 - 自适应填充满窗口
    float panelHeight = windowSize.y * 0.8f;   // 上方设备面板占80%高度
    float debugHeight = windowSize.y * 0.2f;   // 下方调试窗口占20%高度
    float panelWidth = windowSize.x / 3.0f;    // 三个设备面板平均分配宽度
    
    // 上方：三个设备面板并排排列，填充满整个宽度
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

void MainWindow::RenderDebugInfo() {
    // 调试信息显示
    ImGui::Text("调试信息:");
    ImGui::Separator();
    
    // 设备状态
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "身份证阅读器: 未连接");
    ImGui::SameLine();
    
    // 摄像头状态
    bool cameraConnected = false;
    if (m_cameraManager) {
        cameraConnected = m_cameraManager->IsCameraOpen();
    }
    ImGui::TextColored(cameraConnected ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                      cameraConnected ? "摄像头: 已连接" : "摄像头: 未连接");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "手写屏: 未连接");
    
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
    
    // 实时日志显示
    ImGui::Text("实时日志:");
    ImGui::BeginChild("LogDisplay", ImVec2(-1, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
    
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
