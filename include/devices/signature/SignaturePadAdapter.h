#pragma once

#include "devices/interfaces/ISignaturePad.h"
#include <string>
#include <memory>

namespace AsTestTool {

class LibraryLoader;

/**
 * @brief 手写屏适配器类
 */
class SignaturePadAdapter : public ISignaturePad {
public:
    explicit SignaturePadAdapter(const std::string& libraryPath);
    virtual ~SignaturePadAdapter();

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
    std::string m_libraryPath;
    std::unique_ptr<LibraryLoader> m_loader;
    bool m_initialized = false;
    bool m_capturing = false;
};

} // namespace AsTestTool
