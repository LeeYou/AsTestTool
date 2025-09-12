#include "devices/camera/LinuxCamera.h"
#include "utils/Logger.h"
#include "plugins/CameraManager.h"

namespace AsTestTool {

LinuxCamera::LinuxCamera() 
    : m_cameraManager(std::make_unique<Plugins::CameraManager>()) {
    LOG_INFO("LinuxCamera created");
}

LinuxCamera::~LinuxCamera() {
    Shutdown();
}

bool LinuxCamera::Initialize() {
    LOG_INFO("Initializing Linux camera");
    
    if (!m_cameraManager->Initialize()) {
        LOG_ERROR("Failed to initialize camera manager: " + m_cameraManager->GetLastError());
        return false;
    }
    
    m_initialized = true;
    return true;
}

bool LinuxCamera::IsConnected() {
    return m_initialized;
}

void LinuxCamera::Shutdown() {
    LOG_INFO("Shutting down Linux camera");
    StopPreview();
    m_initialized = false;
}

std::string LinuxCamera::GetDeviceInfo() {
    return "Linux Camera";
}

bool LinuxCamera::StartPreview() {
    LOG_INFO("Starting camera preview (Linux implementation)");
    m_previewActive = true;
    return true;
}

bool LinuxCamera::StopPreview() {
    LOG_INFO("Stopping camera preview (Linux implementation)");
    m_previewActive = false;
    return true;
}

bool LinuxCamera::CaptureImage(ImageData& imageData) {
    LOG_INFO("Capturing image (Linux implementation)");
    // TODO: 实现实际的图像捕获逻辑
    return false;
}

std::vector<CameraInfo> LinuxCamera::GetAvailableCameras() {
    if (!m_initialized || !m_cameraManager) {
        LOG_WARNING("Camera not initialized");
        return {};
    }
    
    // 转换插件CameraInfo到设备CameraInfo
    std::vector<Plugins::CameraInfo> pluginCameras = m_cameraManager->GetAvailableCameras();
    std::vector<CameraInfo> cameras;
    
    for (const auto& pluginCamera : pluginCameras) {
        CameraInfo camera;
        camera.name = pluginCamera.name;
        camera.devicePath = pluginCamera.devicePath;
        camera.isAvailable = pluginCamera.isAvailable;
        
        // 转换Resolution类型
        for (const auto& pluginRes : pluginCamera.supportedResolutions) {
            Resolution res(pluginRes.width, pluginRes.height, pluginRes.fps);
            camera.supportedResolutions.push_back(res);
        }
        
        cameras.push_back(camera);
    }
    
    return cameras;
}

bool LinuxCamera::SetResolution(int width, int height) {
    LOG_INFO("Setting camera resolution: " + std::to_string(width) + "x" + std::to_string(height));
    m_currentResolution = Resolution(width, height);
    return true;
}

Resolution LinuxCamera::GetCurrentResolution() {
    return m_currentResolution;
}

bool LinuxCamera::GetPreviewData(ImageData& imageData) {
    // TODO: 实现实际的预览数据获取逻辑
    return false;
}

bool LinuxCamera::SetBrightness(int brightness) {
    LOG_INFO("Setting camera brightness: " + std::to_string(brightness));
    return true;
}

bool LinuxCamera::SetContrast(int contrast) {
    LOG_INFO("Setting camera contrast: " + std::to_string(contrast));
    return true;
}

} // namespace AsTestTool
