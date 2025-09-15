#include "devices/idcard/LibraryLoader.h"
#include "utils/Logger.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace AsTestTool {

LibraryLoader::LibraryLoader() {
    LOG_INFO("LibraryLoader created");
}

LibraryLoader::~LibraryLoader() {
    UnloadDynamicLibrary();
}

bool LibraryLoader::LoadDynamicLibrary(const std::string& path) {
    LOG_INFO("Loading library: " + path);
    
    UnloadDynamicLibrary(); // 先卸载已加载的库
    
#ifdef _WIN32
    m_handle = ::LoadLibraryA(path.c_str());
    if (!m_handle) {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to load library: " + path + ", error code: " + std::to_string(error));
        
        // 获取详细的错误信息
        LPVOID lpMsgBuf;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMsgBuf, 0, NULL);
        
        if (lpMsgBuf) {
            LOG_ERROR("Error details: " + std::string((char*)lpMsgBuf));
            LocalFree(lpMsgBuf);
        }
        
        return false;
    }
#else
    m_handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!m_handle) {
        LOG_ERROR("Failed to load library: " + path + ", error: " + dlerror());
        return false;
    }
#endif
    
    LOG_INFO("Library loaded successfully: " + path);
    return true;
}

void* LibraryLoader::GetFunction(const std::string& name) {
    if (!m_handle) {
        LOG_ERROR("No library loaded");
        return nullptr;
    }
    
#ifdef _WIN32
    void* func = GetProcAddress((HMODULE)m_handle, name.c_str());
    if (!func) {
        LOG_ERROR("Failed to get function: " + name);
    }
    return func;
#else
    void* func = dlsym(m_handle, name.c_str());
    if (!func) {
        LOG_ERROR("Failed to get function: " + name + ", error: " + dlerror());
    }
    return func;
#endif
}

void LibraryLoader::UnloadDynamicLibrary() {
    if (m_handle) {
#ifdef _WIN32
        ::FreeLibrary((HMODULE)m_handle);
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
        LOG_INFO("Library unloaded");
    }
}

bool LibraryLoader::IsLoaded() const {
    return m_handle != nullptr;
}

} // namespace AsTestTool
