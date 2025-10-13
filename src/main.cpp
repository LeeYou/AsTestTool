#include "core/Application.h"
#include "utils/Logger.h"
#include "core/Config.h"
#include <iostream>
#include <exception>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
    // 设置控制台编码（Windows）
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    try {
        // 初始化日志系统
        AsTestTool::Logger::Instance().SetLevel(AsTestTool::LogLevel::Info);
        AsTestTool::Logger::Instance().SetConsoleOutput(true);
        AsTestTool::Logger::Instance().SetOutputFile("astesttool.log");
        
        LOG_INFO("=== AsTestTool Starting ===");
        LOG_INFO("Version: 1.0.0");
        
        // 获取平台信息
        std::string platformName;
#ifdef PLATFORM_WINDOWS
        platformName = "Windows";
#elif PLATFORM_LINUX
        platformName = "Linux";
#else
        platformName = "Unknown";
#endif
        LOG_INFO("Platform: " + platformName);

        // 创建并初始化应用程序
        AsTestTool::Application app;
        
        if (!app.Initialize()) {
            LOG_ERROR("Failed to initialize application");
            std::cerr << "Failed to initialize application. Check log file for details." << std::endl;
            return -1;
        }

        // 运行应用程序
        int result = app.Run();
        
        LOG_INFO("=== AsTestTool Exiting ===");
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        LOG_ERROR("Fatal error: " + std::string(e.what()));
        return -1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        LOG_ERROR("Unknown fatal error occurred");
        return -1;
    }
}
