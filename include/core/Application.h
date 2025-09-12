#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include "devices/interfaces/ICamera.h"
#include "devices/interfaces/ISignaturePad.h"
#include "gui/MainWindow.h"
#include <memory>
#include <string>

// Forward declarations
struct GLFWwindow;

namespace AsTestTool {

/**
 * @brief 应用程序主类
 */
class Application {
public:
    /**
     * @brief 构造函数
     */
    Application();

    /**
     * @brief 析构函数
     */
    ~Application();

    /**
     * @brief 初始化应用程序
     * @return true 初始化成功，false 初始化失败
     */
    bool Initialize();

    /**
     * @brief 运行应用程序
     * @return 应用程序退出码
     */
    int Run();

    /**
     * @brief 关闭应用程序
     */
    void Shutdown();

    /**
     * @brief 获取身份证阅读器
     * @return 身份证阅读器实例
     */
    IIDCardReader* GetIDCardReader() const { return m_idCardReader.get(); }

    /**
     * @brief 获取摄像头
     * @return 摄像头实例
     */
    ICamera* GetCamera() const { return m_camera.get(); }

    /**
     * @brief 获取手写屏
     * @return 手写屏实例
     */
    ISignaturePad* GetSignaturePad() const { return m_signaturePad.get(); }

    /**
     * @brief 检查是否正在运行
     * @return true 正在运行，false 已停止
     */
    bool IsRunning() const { return m_running; }

    /**
     * @brief 设置运行状态
     * @param running 运行状态
     */
    void SetRunning(bool running) { m_running = running; }

private:
    /**
     * @brief 初始化GLFW
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeGLFW();

    /**
     * @brief 关闭GLFW
     */
    void ShutdownGLFW();

    /**
     * @brief 初始化OpenGL
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeOpenGL();

    /**
     * @brief 初始化ImGui
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeImGui();

    /**
     * @brief 关闭ImGui
     */
    void ShutdownImGui();

    /**
     * @brief 处理窗口事件
     */
    void ProcessEvents();

    /**
     * @brief 渲染一帧
     */
    void RenderFrame();

    /**
     * @brief 渲染简单UI
     */
    void RenderSimpleUI();

    /**
     * @brief 设置中文字体
     */
    void SetupChineseFonts();

private:
    /**
     * @brief 初始化设备
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeDevices();

    /**
     * @brief 关闭设备
     */
    void ShutdownDevices();

    /**
     * @brief 初始化GUI
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeGUI();

    /**
     * @brief 关闭GUI
     */
    void ShutdownGUI();

    /**
     * @brief 主循环
     */
    void MainLoop();

    bool m_initialized = false;
    bool m_running = false;
    bool m_guiInitialized = false;
    
    // GLFW and OpenGL
    GLFWwindow* m_window = nullptr;
    int m_windowWidth = 1200;
    int m_windowHeight = 800;
    
    // Devices
    std::unique_ptr<IIDCardReader> m_idCardReader;
    std::unique_ptr<ICamera> m_camera;
    std::unique_ptr<ISignaturePad> m_signaturePad;
    
    // GUI
    std::unique_ptr<MainWindow> m_mainWindow;
};

} // namespace AsTestTool
