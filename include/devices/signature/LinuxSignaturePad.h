#pragma once

#include "devices/interfaces/ISignaturePad.h"
#include <string>

namespace AsTestTool {

/**
 * @brief Linux平台手写屏实现
 */
class LinuxSignaturePad : public ISignaturePad {
public:
    explicit LinuxSignaturePad(const std::string& type = "");
    virtual ~LinuxSignaturePad();

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

private:
    std::string m_type;
    bool m_initialized = false;
    bool m_capturing = false;
};

} // namespace AsTestTool
