#include "devices/signature/SignaturePadAdapter.h"
#include "devices/idcard/LibraryLoader.h"
#include "utils/Logger.h"

namespace AsTestTool {

SignaturePadAdapter::SignaturePadAdapter(const std::string& libraryPath)
    : m_libraryPath(libraryPath) {
    LOG_INFO("SignaturePadAdapter created, library: " + libraryPath);
}

SignaturePadAdapter::~SignaturePadAdapter() {
    Shutdown();
}

bool SignaturePadAdapter::Initialize() {
    LOG_INFO("Initializing signature pad adapter");
    // TODO: 实现适配器初始化逻辑
    m_initialized = true;
    return true;
}

bool SignaturePadAdapter::IsConnected() {
    return m_initialized;
}

void SignaturePadAdapter::Shutdown() {
    LOG_INFO("Shutting down signature pad adapter");
    StopCapture();
    m_initialized = false;
}

std::string SignaturePadAdapter::GetDeviceInfo() {
    return "Signature Pad Adapter - " + m_libraryPath;
}

bool SignaturePadAdapter::StartCapture() {
    LOG_INFO("Starting signature capture via adapter");
    m_capturing = true;
    return true;
}

bool SignaturePadAdapter::StopCapture() {
    LOG_INFO("Stopping signature capture via adapter");
    m_capturing = false;
    return true;
}

bool SignaturePadAdapter::GetSignatureData(SignatureData& data) {
    LOG_INFO("Getting signature data via adapter");
    // TODO: 实现通过适配器获取手写数据的逻辑
    return false;
}

bool SignaturePadAdapter::ClearSignature() {
    LOG_INFO("Clearing signature via adapter");
    // TODO: 实现手写数据清除逻辑
    return true;
}

bool SignaturePadAdapter::GetScreenSize(int& width, int& height) {
    // TODO: 实现屏幕尺寸获取逻辑
    width = 1920;
    height = 1080;
    return true;
}

bool SignaturePadAdapter::SetFullscreen(bool fullscreen) {
    LOG_INFO("Setting fullscreen mode via adapter: " + std::string(fullscreen ? "true" : "false"));
    // TODO: 实现全屏模式设置逻辑
    return true;
}

bool SignaturePadAdapter::IsCapturing() {
    return m_capturing;
}

std::vector<std::string> SignaturePadAdapter::GetSupportedPads() {
    return {"adapter_pad"};
}

} // namespace AsTestTool
