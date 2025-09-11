#pragma once

#include "devices/interfaces/ICamera.h"
#include <memory>

namespace AsTestTool {

/**
 * @brief 摄像头管理器类
 */
class CameraManager : public ICamera {
public:
    CameraManager();
    virtual ~CameraManager();

    // IDevice接口实现
    bool Initialize() override;
    bool IsConnected() override;
    void Shutdown() override;
    std::string GetDeviceInfo() override;

    // ICamera接口实现
    bool StartPreview() override;
    bool StopPreview() override;
    bool CaptureImage(ImageData& imageData) override;
    std::vector<CameraInfo> GetAvailableCameras() override;
    bool SetResolution(int width, int height) override;
    Resolution GetCurrentResolution() override;
    bool GetPreviewData(ImageData& imageData) override;
    bool SetBrightness(int brightness) override;
    bool SetContrast(int contrast) override;

private:
    bool m_initialized = false;
    bool m_previewActive = false;
    Resolution m_currentResolution;
};

} // namespace AsTestTool
