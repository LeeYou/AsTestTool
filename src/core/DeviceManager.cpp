#include "core/DeviceManager.h"
#include "core/DeviceFactory.h"
#include "utils/Logger.h"

namespace AsTestTool {

bool DeviceManager::InitializeAll() {
    LOG_INFO("DeviceManager: Initializing all devices");
    
    if (m_initialized) {
        LOG_WARNING("DeviceManager: Already initialized");
        return true;
    }

    bool allSuccess = true;

    // 初始化身份证阅读器
    if (!InitializeIDCardReader()) {
        allSuccess = false;
    }

    // 初始化摄像头
    if (!InitializeCamera()) {
        allSuccess = false;
    }

    // 初始化手写屏
    if (!InitializeSignaturePad()) {
        allSuccess = false;
    }

    m_initialized = allSuccess;
    
    if (allSuccess) {
        LOG_INFO("DeviceManager: All devices initialized successfully");
    } else {
        LOG_ERROR("DeviceManager: Some devices failed to initialize");
    }

    return allSuccess;
}

void DeviceManager::ShutdownAll() {
    LOG_INFO("DeviceManager: Shutting down all devices");

    ShutdownIDCardReader();
    ShutdownCamera();
    ShutdownSignaturePad();

    m_initialized = false;
    LOG_INFO("DeviceManager: All devices shutdown complete");
}

bool DeviceManager::InitializeIDCardReader() {
    try {
        m_idCardReader = DeviceFactory::CreateIDCardReader("default");
        if (!m_idCardReader) {
            LOG_WARNING("DeviceManager: Failed to create ID card reader");
            return false;
        }

        if (!m_idCardReader->Initialize()) {
            LOG_WARNING("DeviceManager: Failed to initialize ID card reader");
            return false;
        }

        LOG_INFO("DeviceManager: ID card reader initialized successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("DeviceManager: Exception initializing ID card reader: " + std::string(e.what()));
        return false;
    }
}

bool DeviceManager::InitializeCamera() {
    try {
        m_camera = DeviceFactory::CreateCamera();
        if (!m_camera) {
            LOG_WARNING("DeviceManager: Failed to create camera");
            return false;
        }

        if (!m_camera->Initialize()) {
            LOG_WARNING("DeviceManager: Failed to initialize camera");
            return false;
        }

        LOG_INFO("DeviceManager: Camera initialized successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("DeviceManager: Exception initializing camera: " + std::string(e.what()));
        return false;
    }
}

bool DeviceManager::InitializeSignaturePad() {
    try {
        m_signaturePad = DeviceFactory::CreateSignaturePad("default");
        if (!m_signaturePad) {
            LOG_WARNING("DeviceManager: Failed to create signature pad");
            return false;
        }

        if (!m_signaturePad->Initialize()) {
            LOG_WARNING("DeviceManager: Failed to initialize signature pad");
            return false;
        }

        LOG_INFO("DeviceManager: Signature pad initialized successfully");
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("DeviceManager: Exception initializing signature pad: " + std::string(e.what()));
        return false;
    }
}

void DeviceManager::ShutdownIDCardReader() {
    if (m_idCardReader) {
        m_idCardReader->Shutdown();
        m_idCardReader.reset();
        LOG_INFO("DeviceManager: ID card reader shutdown complete");
    }
}

void DeviceManager::ShutdownCamera() {
    if (m_camera) {
        m_camera->Shutdown();
        m_camera.reset();
        LOG_INFO("DeviceManager: Camera shutdown complete");
    }
}

void DeviceManager::ShutdownSignaturePad() {
    if (m_signaturePad) {
        m_signaturePad->Shutdown();
        m_signaturePad.reset();
        LOG_INFO("DeviceManager: Signature pad shutdown complete");
    }
}

bool DeviceManager::IsIDCardReaderConnected() const {
    return m_idCardReader && m_idCardReader->IsConnected();
}

bool DeviceManager::IsCameraConnected() const {
    return m_camera && m_camera->IsConnected();
}

bool DeviceManager::IsSignaturePadConnected() const {
    return m_signaturePad && m_signaturePad->IsConnected();
}

DeviceManager::DeviceStatus DeviceManager::GetStatus() const {
    DeviceStatus status;
    status.idCardConnected = IsIDCardReaderConnected();
    status.cameraConnected = IsCameraConnected();
    status.signatureConnected = IsSignaturePadConnected();
    return status;
}

std::string DeviceManager::GetStatusSummary() const {
    std::string summary;
    
    summary += "ID Card Reader: ";
    summary += IsIDCardReaderConnected() ? "Connected" : "Disconnected";
    summary += " | ";
    
    summary += "Camera: ";
    summary += IsCameraConnected() ? "Connected" : "Disconnected";
    summary += " | ";
    
    summary += "Signature Pad: ";
    summary += IsSignaturePadConnected() ? "Connected" : "Disconnected";
    
    return summary;
}

} // namespace AsTestTool
