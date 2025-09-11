#pragma once

#include <string>
#include <memory>

namespace AsTestTool {

/**
 * @brief 动态库加载器类
 */
class LibraryLoader {
public:
    LibraryLoader();
    ~LibraryLoader();

    /**
     * @brief 加载动态库
     * @param path 库文件路径
     * @return true 加载成功，false 加载失败
     */
    bool LoadDynamicLibrary(const std::string& path);

    /**
     * @brief 获取函数指针
     * @param name 函数名
     * @return 函数指针，nullptr表示失败
     */
    void* GetFunction(const std::string& name);

    /**
     * @brief 卸载动态库
     */
    void UnloadDynamicLibrary();

    /**
     * @brief 检查库是否已加载
     * @return true 已加载，false 未加载
     */
    bool IsLoaded() const;

private:
    void* m_handle = nullptr;
};

} // namespace AsTestTool
