#include "devices/signature/LinuxSignaturePad.h"
#include "utils/Logger.h"

namespace AsTestTool {

LinuxSignaturePad::LinuxSignaturePad(const std::string& type) 
    : m_type(type) {
    LOG_INFO("LinuxSignaturePad created, type: " + type);
}

LinuxSignaturePad::~LinuxSignaturePad() {
    Shutdown();
}

bool LinuxSignaturePad::Initialize() {
    LOG_INFO("Initializing Linux signature pad");
    m_initialized = true;
    return true;
}

bool LinuxSignaturePad::IsConnected() {
    return m_initialized;
}

void LinuxSignaturePad::Shutdown() {
    LOG_INFO("Shutting down Linux signature pad");
    StopCapture();
    m_initialized = false;
}

std::string LinuxSignaturePad::GetDeviceInfo() {
    return "Linux Signature Pad - " + m_type;
}

bool LinuxSignaturePad::StartCapture() {
    LOG_INFO("Starting signature capture (Linux implementation)");
    m_capturing = true;
    return true;
}

bool LinuxSignaturePad::StopCapture() {
    LOG_INFO("Stopping signature capture (Linux implementation)");
    m_capturing = false;
    return true;
}

bool LinuxSignaturePad::GetSignatureData(SignatureData& data) {
    LOG_INFO("Getting signature data (Linux implementation)");
    // TODO: 实现实际的手写数据获取逻辑
    return false;
}

bool LinuxSignaturePad::ClearSignature() {
    LOG_INFO("Clearing signature (Linux implementation)");
    // TODO: 实现实际的手写数据清除逻辑
    return true;
}

bool LinuxSignaturePad::GetScreenSize(int& width, int& height) {
    // TODO: 实现实际的屏幕尺寸获取逻辑
    width = 1920;
    height = 1080;
    return true;
}

bool LinuxSignaturePad::SetFullscreen(bool fullscreen) {
    LOG_INFO("Setting fullscreen mode: " + std::string(fullscreen ? "true" : "false"));
    // TODO: 实现实际的全屏模式设置逻辑
    return true;
}

bool LinuxSignaturePad::IsCapturing() {
    return m_capturing;
}

std::vector<std::string> LinuxSignaturePad::GetSupportedPads() {
    return {"default", "pad1", "pad2"};
}

} // namespace AsTestTool
