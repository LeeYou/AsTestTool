#pragma once

#include "devices/interfaces/ISignaturePad.h"
#include "devices/idcard/LibraryLoader.h"
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

namespace AsTestTool {

// DLL接口数据结构
struct AXIS {
    LONG axMin;
    LONG axMax;
};

struct PACKETS {
    int btn;
    LONG X;
    LONG Y;
    LONG Press;
};

// DLL接口宏定义
#define HW_GET_STATUS 1
#define HW_GET_AXIS_X 2
#define HW_GET_AXIS_Y 3
#define HW_GET_PRESSURE 4
#define HW_GET_VENDOR 5
#define HW_GET_PRODUCT 6
#define HW_GET_DRV_VER 7
#define HW_CLOSE 0xffffffff

#define SUCCESS 0

/**
 * @brief Windows平台手写屏实现
 */
class WindowsSignaturePad : public ISignaturePad {
public:
    explicit WindowsSignaturePad(const std::string& type = "");
    virtual ~WindowsSignaturePad();

    // IDevice接口实现
    bool Initialize() override;
    bool IsConnected() override;
    void Shutdown() override;
    std::string GetDeviceInfo() override;

    // ISignaturePad接口实现
    bool StartCapture() override;
    bool StopCapture() override;
    bool GetSignatureData(SignatureData& data) override;
    bool ClearSignature() override;
    bool GetScreenSize(int& width, int& height) override;
    bool SetFullscreen(bool fullscreen) override;
    bool IsCapturing() override;
    std::vector<std::string> GetSupportedPads() override;

    // DLL相关方法
    bool LoadLibrary(const std::string& libraryPath);
    bool OpenDevice();
    bool CloseDevice();
    int GetDeviceInfo(int nIndex, LPVOID lpOutput);
    int GetPackets(PACKETS& packets);
    int GetGesture(HWND hWnd, UINT uMsg, bool bGesture);

private:
    std::string m_type;
    bool m_initialized = false;
    bool m_capturing = false;
    bool m_deviceOpened = false;
    bool m_fullscreen = false;
    
    // DLL相关
    std::unique_ptr<LibraryLoader> m_libraryLoader;
    
    // DLL函数指针类型定义
    typedef int (WINAPI* OpenDeviceFunc)();
    typedef int (WINAPI* GetDeviceInfoFunc)(int nIndex, LPVOID lpOutput);
    typedef int (WINAPI* GetPacketsFunc)(PACKETS& packets);
    typedef int (WINAPI* GetGestureFunc)(HWND hWnd, UINT uMsg, bool bGesture);
    
    // DLL函数指针
    OpenDeviceFunc m_openDeviceFunc = nullptr;
    GetDeviceInfoFunc m_getDeviceInfoFunc = nullptr;
    GetPacketsFunc m_getPacketsFunc = nullptr;
    GetGestureFunc m_getGestureFunc = nullptr;
    
    // 设备信息
    AXIS m_axisX, m_axisY;
    LONG m_pressureLevel = 0;
    std::string m_vendor, m_product;
    DWORD m_driverVersion = 0;
    
    // 手写数据
    SignatureData m_signatureData;
    std::atomic<bool> m_keepReading{false};
    std::thread m_captureThread;
    
    // 内部方法
    void CaptureLoop();
    bool ParseDeviceInfo();
    void ConvertPacketsToSignaturePoint(const PACKETS& packets, SignaturePoint& point);
};

} // namespace AsTestTool
