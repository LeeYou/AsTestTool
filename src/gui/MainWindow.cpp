#include "gui/MainWindow.h"
#include "utils/Logger.h"
#include "gui/IDCardPanel.h"
#include "gui/CameraPanel.h"
#include "gui/SignaturePanel.h"
#include "gui/Theme.h"

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
    
    // 创建各个面板
    m_idCardPanel = std::make_unique<IDCardPanel>();
    m_cameraPanel = std::make_unique<CameraPanel>();
    m_signaturePanel = std::make_unique<SignaturePanel>();
    
    // 应用亮色主题
    Theme::Instance().ApplyLightTheme();
    
    m_initialized = true;
    return true;
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
    
    // 渲染菜单栏
    RenderMenuBar();
    
    // 渲染主内容
    RenderMainContent();
    
    // 渲染状态栏
    RenderStatusBar();
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
    // 创建三个并排的窗口
    // ImGui::SetNextWindowDockID(ImGui::GetID("MyDockSpace"), ImGuiCond_FirstUseEver);
    
    // 身份证阅读器面板
    if (ImGui::Begin("身份证阅读器", nullptr, ImGuiWindowFlags_None)) {
        if (m_idCardPanel) {
            m_idCardPanel->Render();
        }
    }
    ImGui::End();
    
    // 摄像头面板
    if (ImGui::Begin("摄像头/高拍仪", nullptr, ImGuiWindowFlags_None)) {
        if (m_cameraPanel) {
            m_cameraPanel->Render();
        }
    }
    ImGui::End();
    
    // 手写屏面板
    if (ImGui::Begin("手写屏", nullptr, ImGuiWindowFlags_None)) {
        if (m_signaturePanel) {
            m_signaturePanel->Render();
        }
    }
    ImGui::End();
}

void MainWindow::RenderStatusBar() {
    // 简化的状态栏
    ImGui::Separator();
    ImGui::Text("就绪 | 设备状态: 未连接 | 版本: 1.0.0 | AsTestTool v1.0");
}

} // namespace AsTestTool
