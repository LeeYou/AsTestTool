#include "core/Application.h"
#include "core/DeviceFactory.h"
#include "core/Config.h"
#include "utils/Logger.h"
#include "utils/LogDisplay.h"
#include "core/ErrorCode.h"
#include "gui/MainWindow.h"
#include <thread>
#include <chrono>
#include <filesystem>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>

namespace AsTestTool {

Application::Application() {
    LOG_INFO("Application constructor called");
}

Application::~Application() {
    LOG_INFO("Application destructor called");
    Shutdown();
}

bool Application::Initialize() {
    LOG_INFO("Initializing application");
    
    if (m_initialized) {
        LOG_WARNING("Application already initialized");
        return true;
    }

    try {
        // 设置默认配置
        Config::Instance().SetDefaults();
        
        // 设置Logger回调，将日志转发到LogDisplay
        Logger::Instance().SetLogCallback([](LogLevel level, const std::string& message) {
            LogDisplayLevel displayLevel;
            switch (level) {
                case LogLevel::Debug: displayLevel = LogDisplayLevel::Debug; break;
                case LogLevel::Info: displayLevel = LogDisplayLevel::Info; break;
                case LogLevel::Warning: displayLevel = LogDisplayLevel::Warning; break;
                case LogLevel::Error: displayLevel = LogDisplayLevel::Error; break;
                default: displayLevel = LogDisplayLevel::Info; break;
            }
            LogDisplay::Instance().AddLog(displayLevel, message);
        });
        
        // 初始化设备
        if (!InitializeDevices()) {
            LOG_ERROR("Failed to initialize devices");
            return false;
        }
        
        // 初始化GUI
        if (!InitializeGUI()) {
            LOG_ERROR("Failed to initialize GUI");
            return false;
        }
        
        m_initialized = true;
        LOG_INFO("Application initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during initialization: " + std::string(e.what()));
        return false;
    }
}

int Application::Run() {
    LOG_INFO("Starting application main loop");
    
    if (!m_initialized) {
        LOG_ERROR("Application not initialized");
        return -1;
    }
    
    m_running = true;
    
    try {
        MainLoop();
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in main loop: " + std::string(e.what()));
        return -1;
    }
    
    LOG_INFO("Application main loop ended");
    return 0;
}

void Application::Shutdown() {
    LOG_INFO("Shutting down application");
    
    m_running = false;
    
    ShutdownDevices();
    ShutdownGUI();
    
    m_initialized = false;
    LOG_INFO("Application shutdown complete");
}

bool Application::InitializeDevices() {
    LOG_INFO("Initializing devices");
    
    try {
        // 创建身份证阅读器
        m_idCardReader = DeviceFactory::CreateIDCardReader();
        if (!m_idCardReader) {
            LOG_WARNING("Failed to create ID card reader");
        }
        
        // 创建摄像头
        m_camera = DeviceFactory::CreateCamera();
        if (!m_camera) {
            LOG_WARNING("Failed to create camera");
        }
        
        // 创建手写屏
        m_signaturePad = DeviceFactory::CreateSignaturePad();
        if (!m_signaturePad) {
            LOG_WARNING("Failed to create signature pad");
        }
        
        LOG_INFO("Devices initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during device initialization: " + std::string(e.what()));
        return false;
    }
}

void Application::ShutdownDevices() {
    LOG_INFO("Shutting down devices");
    
    if (m_idCardReader) {
        m_idCardReader->Shutdown();
        m_idCardReader.reset();
    }
    
    if (m_camera) {
        m_camera->Shutdown();
        m_camera.reset();
    }
    
    if (m_signaturePad) {
        m_signaturePad->Shutdown();
        m_signaturePad.reset();
    }
    
    LOG_INFO("Devices shutdown complete");
}

bool Application::InitializeGUI() {
    LOG_INFO("Initializing GUI");
    
    // 初始化GLFW
    if (!InitializeGLFW()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    // 初始化OpenGL
    if (!InitializeOpenGL()) {
        LOG_ERROR("Failed to initialize OpenGL");
        return false;
    }
    
    // 初始化ImGui
    if (!InitializeImGui()) {
        LOG_ERROR("Failed to initialize ImGui");
        return false;
    }
    
    // 创建主窗口
    m_mainWindow = std::make_unique<MainWindow>();
    if (!m_mainWindow->Initialize()) {
        LOG_ERROR("Failed to initialize main window");
        return false;
    }
    
    LOG_INFO("GUI initialized successfully");
    return true;
}

void Application::ShutdownGUI() {
    LOG_INFO("Shutting down GUI");
    
    // 清理主窗口
    if (m_mainWindow) {
        m_mainWindow.reset();
    }
    
    // 关闭ImGui
    ShutdownImGui();
    
    // 关闭GLFW
    ShutdownGLFW();
    
    LOG_INFO("GUI shutdown complete");
}

void Application::MainLoop() {
    LOG_INFO("Entering main loop");
    
    while (m_running && !glfwWindowShouldClose(m_window)) {
        // 处理事件
        ProcessEvents();
        
        // 开始新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // 渲染主窗口
        if (m_mainWindow) {
            m_mainWindow->Render();
        }
        
        // 渲染ImGui
        ImGui::Render();
        
        // 渲染到屏幕
        RenderFrame();
        
        // 交换缓冲区
        glfwSwapBuffers(m_window);
    }
    
    LOG_INFO("Exiting main loop");
}

bool Application::InitializeGLFW() {
    LOG_INFO("Initializing GLFW");
    
    // 初始化GLFW
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建窗口
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "AsTestTool v1.0", nullptr, nullptr);
    if (!m_window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    
    // 设置窗口为最大化模式
    glfwMaximizeWindow(m_window);
    
    // 设置上下文
    glfwMakeContextCurrent(m_window);
    
    // 设置垂直同步
    glfwSwapInterval(1);
    
    // 设置窗口关闭回调
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        // 可以在这里添加清理逻辑
    });
    
    LOG_INFO("GLFW initialized successfully");
    return true;
}

void Application::ShutdownGLFW() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    LOG_INFO("GLFW shutdown complete");
}

bool Application::InitializeOpenGL() {
    LOG_INFO("Initializing OpenGL");
    
    // 设置视口
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 设置清除颜色（亮色主题背景）
    glClearColor(0.96f, 0.96f, 0.96f, 1.0f);
    
    LOG_INFO("OpenGL initialized successfully");
    return true;
}

bool Application::InitializeImGui() {
    LOG_INFO("Initializing ImGui");
    
    // 设置ImGui上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // 注释掉，可能不支持
    
    // 设置ImGui样式（亮色主题）
    ImGui::StyleColorsLight();
    
    // 初始化ImGui平台/渲染器绑定
    if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) {
        LOG_ERROR("Failed to initialize ImGui GLFW binding");
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        LOG_ERROR("Failed to initialize ImGui OpenGL3 binding");
        return false;
    }
    
    // 在渲染器初始化后设置中文字体
    SetupChineseFonts();
    
    LOG_INFO("ImGui initialized successfully");
    return true;
}

void Application::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    LOG_INFO("ImGui shutdown complete");
}

void Application::ProcessEvents() {
    glfwPollEvents();
}

void Application::RenderFrame() {
    // 清除屏幕
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 渲染ImGui
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::RenderSimpleUI() {
    // 渲染一个简单的界面
    if (ImGui::Begin("AsTestTool - 设备测试工具", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("欢迎使用 AsTestTool v1.0");
        ImGui::Separator();
        
        // 菜单栏
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("设备")) {
                if (ImGui::MenuItem("身份证阅读器")) {
                    LOG_INFO("身份证阅读器菜单被点击");
                }
                if (ImGui::MenuItem("摄像头")) {
                    LOG_INFO("摄像头菜单被点击");
                }
                if (ImGui::MenuItem("手写屏")) {
                    LOG_INFO("手写屏菜单被点击");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("帮助")) {
                if (ImGui::MenuItem("关于")) {
                    LOG_INFO("关于菜单被点击");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        // 设备状态
        ImGui::Text("设备状态:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "未连接");
        
        ImGui::Separator();
        
        // 身份证阅读器
        if (ImGui::CollapsingHeader("身份证阅读器", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("连接设备", ImVec2(100, 30))) {
                LOG_INFO("连接身份证阅读器设备");
            }
            ImGui::SameLine();
            if (ImGui::Button("读取身份证", ImVec2(100, 30))) {
                LOG_INFO("读取身份证");
            }
            ImGui::Text("设备状态: 未连接");
            ImGui::Text("身份证信息: 未读取");
        }
        
        // 摄像头
        if (ImGui::CollapsingHeader("摄像头/高拍仪", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("选择设备", ImVec2(100, 30))) {
                LOG_INFO("选择摄像头设备");
            }
            ImGui::SameLine();
            if (ImGui::Button("开始预览", ImVec2(100, 30))) {
                LOG_INFO("开始摄像头预览");
            }
            ImGui::Text("设备状态: 未连接");
            ImGui::Text("预览: 未启动");
        }
        
        // 手写屏
        if (ImGui::CollapsingHeader("手写屏", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("连接设备", ImVec2(100, 30))) {
                LOG_INFO("连接手写屏设备");
            }
            ImGui::SameLine();
            if (ImGui::Button("开始测试", ImVec2(100, 30))) {
                LOG_INFO("开始手写屏测试");
            }
            ImGui::Text("设备状态: 未连接");
            ImGui::Text("手写轨迹: 无数据");
        }
        
        ImGui::Separator();
        ImGui::Text("就绪 | 版本: 1.0.0 | AsTestTool");
    }
    ImGui::End();
}

void Application::SetupChineseFonts() {
    LOG_INFO("Setting up Chinese fonts");
    
    ImGuiIO& io = ImGui::GetIO();
    
    // 尝试加载系统中文字体
    const char* fontPaths[] = {
        "C:/Windows/Fonts/msyh.ttc",      // 微软雅黑
        "C:/Windows/Fonts/simsun.ttc",    // 宋体
        "C:/Windows/Fonts/simhei.ttf",    // 黑体
        "C:/Windows/Fonts/simkai.ttf",    // 楷体
        "/System/Library/Fonts/PingFang.ttc",  // macOS
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",  // Linux
        nullptr
    };
    
    ImFont* chineseFont = nullptr;
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        if (std::filesystem::exists(fontPaths[i])) {
            LOG_INFO("Loading Chinese font: " + std::string(fontPaths[i]));
            chineseFont = io.Fonts->AddFontFromFileTTF(fontPaths[i], 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
            if (chineseFont) {
                LOG_INFO("Chinese font loaded successfully");
                break;
            } else {
                LOG_WARNING("Failed to load font: " + std::string(fontPaths[i]));
            }
        } else {
            LOG_INFO("Font not found: " + std::string(fontPaths[i]));
        }
    }
    
    // 如果系统字体加载失败，使用默认字体
    if (!chineseFont) {
        LOG_WARNING("Failed to load Chinese font, using default font");
        chineseFont = io.Fonts->AddFontDefault();
    }
    
    LOG_INFO("Font setup completed");
}

} // namespace AsTestTool
