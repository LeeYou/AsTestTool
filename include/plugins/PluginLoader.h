#pragma once
#include "ICameraPlugin.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace AsTestTool {
namespace Plugins {

class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader();
    
    // 插件管理
    bool LoadPlugin(const std::string& pluginPath);
    bool UnloadPlugin(const std::string& pluginName);
    void UnloadAllPlugins();
    
    // 插件查询
    std::vector<std::string> GetLoadedPlugins() const;
    std::unique_ptr<ICameraPlugin> CreatePlugin(const std::string& pluginName);
    bool IsPluginLoaded(const std::string& pluginName) const;
    
    // 自动发现
    bool AutoDiscoverPlugins(const std::string& pluginDirectory);
    
    // 错误处理
    std::string GetLastError() const;
    int GetLastErrorCode() const;
    
private:
    struct PluginInfo {
        void* handle;
        std::unique_ptr<IPluginFactory> factory;
        std::string path;
        std::string name;
        
        PluginInfo() : handle(nullptr) {}
        PluginInfo(void* h, std::unique_ptr<IPluginFactory> f, const std::string& p, const std::string& n)
            : handle(h), factory(std::move(f)), path(p), name(n) {}
        
        // 移动构造函数
        PluginInfo(PluginInfo&& other) noexcept
            : handle(other.handle), factory(std::move(other.factory)), 
              path(std::move(other.path)), name(std::move(other.name)) {
            other.handle = nullptr;
        }
        
        // 移动赋值操作符
        PluginInfo& operator=(PluginInfo&& other) noexcept {
            if (this != &other) {
                handle = other.handle;
                factory = std::move(other.factory);
                path = std::move(other.path);
                name = std::move(other.name);
                other.handle = nullptr;
            }
            return *this;
        }
        
        // 禁用拷贝
        PluginInfo(const PluginInfo&) = delete;
        PluginInfo& operator=(const PluginInfo&) = delete;
    };
    
    std::unordered_map<std::string, PluginInfo> m_plugins;
    std::string m_lastError;
    int m_lastErrorCode;
    
    // 动态库加载
    void* LoadLibraryImpl(const std::string& path);
    void UnloadLibraryImpl(void* handle);
    void* GetFunction(void* handle, const std::string& functionName);
    
    // 错误处理
    void SetLastError(const std::string& error, int code = -1);
    
    // 插件名称提取
    std::string ExtractPluginName(const std::string& pluginPath) const;
    
    // 平台特定实现
#ifdef _WIN32
    static constexpr const char* PLUGIN_EXTENSION = ".dll";
    static constexpr const char* CREATE_FACTORY_FUNC = "CreatePluginFactory";
    static constexpr const char* DESTROY_FACTORY_FUNC = "DestroyPluginFactory";
#else
    static constexpr const char* PLUGIN_EXTENSION = ".so";
    static constexpr const char* CREATE_FACTORY_FUNC = "CreatePluginFactory";
    static constexpr const char* DESTROY_FACTORY_FUNC = "DestroyPluginFactory";
#endif
};

} // namespace Plugins
} // namespace AsTestTool
