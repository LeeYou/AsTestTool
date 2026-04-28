#pragma once

#include <string>
#include <vector>
#include <memory>

namespace AsTestTool {

/**
 * @brief 文件对话框过滤器
 */
struct FileDialogFilter {
    std::string name;       // 过滤器名称，如 "DLL Files"
    std::string pattern;    // 过滤器模式，如 "*.dll"
};

/**
 * @brief 文件对话框结果
 */
struct FileDialogResult {
    bool success = false;           // 是否成功
    std::string filePath;          // 选择的文件路径
    std::string fileName;          // 文件名
    std::string directory;          // 目录路径
};

/**
 * @brief 文件对话框抽象类
 * 
 * 跨平台文件对话框抽象，解决 Windows API 直接使用问题。
 */
class FileDialog {
public:
    virtual ~FileDialog() = default;

    /**
     * @brief 打开文件对话框
     * @param title 对话框标题
     * @param filters 过滤器列表
     * @param defaultPath 默认路径
     * @return 对话框结果
     */
    virtual FileDialogResult OpenFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultPath = ""
    ) = 0;

    /**
     * @brief 保存文件对话框
     * @param title 对话框标题
     * @param filters 过滤器列表
     * @param defaultName 默认文件名
     * @return 对话框结果
     */
    virtual FileDialogResult SaveFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultName = ""
    ) = 0;

    /**
     * @brief 选择目录对话框
     * @param title 对话框标题
     * @param defaultPath 默认路径
     * @return 对话框结果
     */
    virtual FileDialogResult SelectDirectory(
        const std::string& title,
        const std::string& defaultPath = ""
    ) = 0;

    /**
     * @brief 创建平台特定的 FileDialog 实例
     */
    static std::unique_ptr<FileDialog> Create();
};

// Windows 实现
#ifdef _WIN32
class WindowsFileDialog : public FileDialog {
public:
    FileDialogResult OpenFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultPath = ""
    ) override;

    FileDialogResult SaveFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultName = ""
    ) override;

    FileDialogResult SelectDirectory(
        const std::string& title,
        const std::string& defaultPath = ""
    ) override;
};
#endif

// Linux 实现
#ifdef __linux__
class LinuxFileDialog : public FileDialog {
public:
    FileDialogResult OpenFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultPath = ""
    ) override;

    FileDialogResult SaveFile(
        const std::string& title,
        const std::vector<FileDialogFilter>& filters,
        const std::string& defaultName = ""
    ) override;

    FileDialogResult SelectDirectory(
        const std::string& title,
        const std::string& defaultPath = ""
    ) override;
};
#endif

} // namespace AsTestTool
