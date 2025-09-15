#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include "devices/idcard/LibraryLoader.h"
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

namespace AsTestTool {

/**
 * @brief Windows平台身份证阅读器实现
 * 基于标准身份证阅读器DLL接口实现
 */
class WindowsIDCardReader : public IIDCardReader {
public:
    explicit WindowsIDCardReader(const std::string& type = "");
    virtual ~WindowsIDCardReader();

    // IDevice接口实现
    bool Initialize() override;
    bool IsConnected() override;
    void Shutdown() override;
    std::string GetDeviceInfo() override;

    // IIDCardReader接口实现
    bool ReadCard(IDCardInfo& info) override;
    std::vector<std::string> GetSupportedReaders() override;
    bool HasCard() override;
    bool EjectCard() override;

    // 设备管理
    bool OpenDevice(int port = 1001);
    bool CloseDevice();
    int GetDeviceStatus();
    bool LoadLibrary(const std::string& libraryPath);

private:
    // DLL函数指针类型定义
    typedef int (WINAPI* OpenDeviceFunc)(int port, char* outMessage);
    typedef int (WINAPI* ReadCardFunc)(char* outMessage);
    typedef int (WINAPI* GetDeviceStatusFunc)(char* outMessage);
    typedef int (WINAPI* GetCardInfoFunc)(int index, char* outValue, char* outMessage);
    typedef int (WINAPI* CloseDeviceFunc)(char* outMessage);

    // DLL函数指针
    OpenDeviceFunc m_openDeviceFunc = nullptr;
    ReadCardFunc m_readCardFunc = nullptr;
    GetDeviceStatusFunc m_getDeviceStatusFunc = nullptr;
    GetCardInfoFunc m_getCardInfoFunc = nullptr;
    CloseDeviceFunc m_closeDeviceFunc = nullptr;

    // 设备状态
    std::string m_type;
    bool m_initialized = false;
    bool m_deviceOpened = false;
    int m_devicePort = 1001;
    
    // DLL加载器
    std::unique_ptr<LibraryLoader> m_libraryLoader;
    
    // 错误处理
    std::string GetLastError() const;
    bool CheckDeviceStatus();
    
    // 身份证信息解析
    bool ParseCardInfo(IDCardInfo& info);
    std::string GetCardInfoByIndex(int index);
};

} // namespace AsTestTool
