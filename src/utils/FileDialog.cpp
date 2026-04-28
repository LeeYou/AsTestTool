#include "utils/FileDialog.h"
#include "utils/Logger.h"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#endif

namespace AsTestTool {

std::unique_ptr<FileDialog> FileDialog::Create() {
#ifdef _WIN32
    return std::make_unique<WindowsFileDialog>();
#elif __linux__
    return std::make_unique<LinuxFileDialog>();
#else
    return nullptr;
#endif
}

#ifdef _WIN32

FileDialogResult WindowsFileDialog::OpenFile(
    const std::string& title,
    const std::vector<FileDialogFilter>& filters,
    const std::string& defaultPath
) {
    FileDialogResult result;

    // 构建过滤器字符串
    std::string filterStr;
    for (const auto& f : filters) {
        filterStr += f.name + '\0' + f.pattern + '\0';
    }
    filterStr += '\0';

    OPENFILENAMEA ofn = {};
    char filePath[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.lpstrFilter = filterStr.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (!defaultPath.empty()) {
        ofn.lpstrInitialDir = defaultPath.c_str();
    }

    if (GetOpenFileNameA(&ofn)) {
        result.success = true;
        result.filePath = filePath;

        // 提取文件名和目录
        size_t lastSep = result.filePath.find_last_of("/\\");
        if (lastSep != std::string::npos) {
            result.fileName = result.filePath.substr(lastSep + 1);
            result.directory = result.filePath.substr(0, lastSep);
        } else {
            result.fileName = result.filePath;
        }
    }

    return result;
}

FileDialogResult WindowsFileDialog::SaveFile(
    const std::string& title,
    const std::vector<FileDialogFilter>& filters,
    const std::string& defaultName
) {
    FileDialogResult result;

    // 构建过滤器字符串
    std::string filterStr;
    for (const auto& f : filters) {
        filterStr += f.name + '\0' + f.pattern + '\0';
    }
    filterStr += '\0';

    OPENFILENAMEA ofn = {};
    char filePath[MAX_PATH] = {};
    
    if (!defaultName.empty()) {
        strncpy_s(filePath, defaultName.c_str(), MAX_PATH - 1);
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title.c_str();
    ofn.lpstrFilter = filterStr.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        result.success = true;
        result.filePath = filePath;

        // 提取文件名和目录
        size_t lastSep = result.filePath.find_last_of("/\\");
        if (lastSep != std::string::npos) {
            result.fileName = result.filePath.substr(lastSep + 1);
            result.directory = result.filePath.substr(0, lastSep);
        } else {
            result.fileName = result.filePath;
        }
    }

    return result;
}

FileDialogResult WindowsFileDialog::SelectDirectory(
    const std::string& title,
    const std::string& defaultPath
) {
    FileDialogResult result;

    char folderPath[MAX_PATH] = {};

    BROWSEINFOA bi = {};
    bi.lpszTitle = title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.pidlRoot = nullptr;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl) {
        if (SHGetPathFromIDListA(pidl, folderPath)) {
            result.success = true;
            result.filePath = folderPath;
            result.directory = folderPath;
        }
        
        // 释放 PIDL
        IMalloc* imalloc = nullptr;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }

    return result;
}

#endif // _WIN32

#ifdef __linux__

FileDialogResult LinuxFileDialog::OpenFile(
    const std::string& title,
    const std::vector<FileDialogFilter>& filters,
    const std::string& defaultPath
) {
    // Linux 平台暂时使用简单的实现
    // 实际应用中可以使用 zenity, kdialog 或 GTK/Qt 对话框
    FileDialogResult result;
    LOG_WARNING("LinuxFileDialog::OpenFile not fully implemented, using fallback");
    
    // 这里可以集成 zenity 或其他 Linux 文件对话框工具
    // 示例: 使用 zenity --file-selection
    // system("zenity --file-selection --title=\"Select File\"");
    
    return result;
}

FileDialogResult LinuxFileDialog::SaveFile(
    const std::string& title,
    const std::vector<FileDialogFilter>& filters,
    const std::string& defaultName
) {
    FileDialogResult result;
    LOG_WARNING("LinuxFileDialog::SaveFile not fully implemented");
    return result;
}

FileDialogResult LinuxFileDialog::SelectDirectory(
    const std::string& title,
    const std::string& defaultPath
) {
    FileDialogResult result;
    LOG_WARNING("LinuxFileDialog::SelectDirectory not fully implemented");
    return result;
}

#endif // __linux__

} // namespace AsTestTool
