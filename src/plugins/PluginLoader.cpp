#include "plugins/PluginLoader.h"
#include "utils/Logger.h"
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace AsTestTool {
namespace Plugins {

PluginLoader::PluginLoader() : m_lastErrorCode(0) {
}

PluginLoader::~PluginLoader() {
    UnloadAllPlugins();
}

bool PluginLoader::LoadPlugin(const std::string& pluginPath) {
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(pluginPath)) {
            SetLastError("Plugin file does not exist: " + pluginPath, -1);
            return false;
        }
        
        // 提取插件名称
        std::string pluginName = ExtractPluginName(pluginPath);
        
        // 检查是否已经加载
        if (IsPluginLoaded(pluginName)) {
            SetLastError("Plugin already loaded: " + pluginName, -2);
            return false;
        }
        
        // 加载动态库
        void* handle = LoadLibraryImpl(pluginPath);
        if (!handle) {
            SetLastError("Failed to load library: " + pluginPath, -3);
            return false;
        }
        
        // 获取工厂创建函数
        CreateFactoryFunc createFactory = reinterpret_cast<CreateFactoryFunc>(
            GetFunction(handle, CREATE_FACTORY_FUNC));
        if (!createFactory) {
            UnloadLibraryImpl(handle);
            SetLastError("Failed to get CreatePluginFactory function from: " + pluginPath, -4);
            return false;
        }
        
        // 创建工厂实例
        IPluginFactory* factory = createFactory();
        if (!factory) {
            UnloadLibraryImpl(handle);
            SetLastError("Failed to create plugin factory from: " + pluginPath, -5);
            return false;
        }
        
        // 存储插件信息
        m_plugins[pluginName] = PluginInfo(handle, std::unique_ptr<IPluginFactory>(factory), pluginPath, pluginName);
        
        Logger::Instance().Log(LogLevel::Info, "Successfully loaded plugin: " + pluginName + " from " + pluginPath);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while loading plugin: " + std::string(e.what()), -6);
        return false;
    }
}

bool PluginLoader::UnloadPlugin(const std::string& pluginName) {
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end()) {
        SetLastError("Plugin not found: " + pluginName, -7);
        return false;
    }
    
    try {
        // 获取销毁函数
        DestroyFactoryFunc destroyFactory = reinterpret_cast<DestroyFactoryFunc>(
            GetFunction(it->second.handle, DESTROY_FACTORY_FUNC));
        
        // 销毁工厂（如果存在销毁函数）
        if (destroyFactory && it->second.factory) {
            destroyFactory(it->second.factory.release());
        }
        
        // 卸载动态库
        UnloadLibraryImpl(it->second.handle);
        
        // 从映射中移除
        m_plugins.erase(it);
        
        Logger::Instance().Log(LogLevel::Info, "Successfully unloaded plugin: " + pluginName);
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while unloading plugin: " + std::string(e.what()), -8);
        return false;
    }
}

void PluginLoader::UnloadAllPlugins() {
    std::vector<std::string> pluginNames;
    for (const auto& pair : m_plugins) {
        pluginNames.push_back(pair.first);
    }
    
    for (const auto& name : pluginNames) {
        UnloadPlugin(name);
    }
}

std::vector<std::string> PluginLoader::GetLoadedPlugins() const {
    std::vector<std::string> plugins;
    for (const auto& pair : m_plugins) {
        plugins.push_back(pair.first);
    }
    return plugins;
}

std::unique_ptr<ICameraPlugin> PluginLoader::CreatePlugin(const std::string& pluginName) {
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end()) {
        SetLastError("Plugin not found: " + pluginName, -9);
        return nullptr;
    }
    
    if (!it->second.factory) {
        SetLastError("Plugin factory is null: " + pluginName, -10);
        return nullptr;
    }
    
    try {
        return it->second.factory->CreatePlugin();
    } catch (const std::exception& e) {
        SetLastError("Exception while creating plugin instance: " + std::string(e.what()), -11);
        return nullptr;
    }
}

bool PluginLoader::IsPluginLoaded(const std::string& pluginName) const {
    return m_plugins.find(pluginName) != m_plugins.end();
}

bool PluginLoader::AutoDiscoverPlugins(const std::string& pluginDirectory) {
    try {
        if (!std::filesystem::exists(pluginDirectory)) {
            SetLastError("Plugin directory does not exist: " + pluginDirectory, -12);
            return false;
        }
        
        bool foundAny = false;
        for (const auto& entry : std::filesystem::directory_iterator(pluginDirectory)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                std::string extension = entry.path().extension().string();
                
                // 检查文件扩展名
                if (extension == PLUGIN_EXTENSION) {
                    if (LoadPlugin(path)) {
                        foundAny = true;
                    }
                }
            }
        }
        
        return foundAny;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while auto-discovering plugins: " + std::string(e.what()), -13);
        return false;
    }
}

std::string PluginLoader::GetLastError() const {
    return m_lastError;
}

int PluginLoader::GetLastErrorCode() const {
    return m_lastErrorCode;
}

void* PluginLoader::LoadLibraryImpl(const std::string& path) {
#ifdef _WIN32
    return ::LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void PluginLoader::UnloadLibraryImpl(void* handle) {
    if (!handle) return;
    
#ifdef _WIN32
    ::FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

void* PluginLoader::GetFunction(void* handle, const std::string& functionName) {
    if (!handle) return nullptr;
    
#ifdef _WIN32
    return ::GetProcAddress(static_cast<HMODULE>(handle), functionName.c_str());
#else
    return dlsym(handle, functionName.c_str());
#endif
}

void PluginLoader::SetLastError(const std::string& error, int code) {
    m_lastError = error;
    m_lastErrorCode = code;
    Logger::Instance().Log(LogLevel::Error, "PluginLoader Error [" + std::to_string(code) + "]: " + error);
}

std::string PluginLoader::ExtractPluginName(const std::string& pluginPath) const {
    std::filesystem::path path(pluginPath);
    std::string filename = path.filename().string();
    
    // 移除扩展名
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }
    
    return filename;
}

} // namespace Plugins
} // namespace AsTestTool
