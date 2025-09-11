#include "devices/signature/FullscreenWindow.h"
#include "devices/interfaces/ISignaturePad.h"
#include "utils/Logger.h"

namespace AsTestTool {

FullscreenWindow::FullscreenWindow() {
    LOG_INFO("FullscreenWindow created");
}

FullscreenWindow::~FullscreenWindow() {
    Close();
}

bool FullscreenWindow::Create(int width, int height) {
    LOG_INFO("Creating fullscreen window: " + std::to_string(width) + "x" + std::to_string(height));
    m_width = width;
    m_height = height;
    m_created = true;
    return true;
}

void FullscreenWindow::Show() {
    LOG_INFO("Showing fullscreen window");
    m_visible = true;
}

void FullscreenWindow::Hide() {
    LOG_INFO("Hiding fullscreen window");
    m_visible = false;
}

void FullscreenWindow::DrawSignature(const std::vector<SignaturePoint>& points) {
    // TODO: 实现手写轨迹绘制逻辑
}

bool FullscreenWindow::IsVisible() const {
    return m_visible;
}

void FullscreenWindow::Update() {
    // TODO: 实现窗口更新逻辑
}

void FullscreenWindow::Render() {
    // TODO: 实现窗口渲染逻辑
}

void FullscreenWindow::Close() {
    LOG_INFO("Closing fullscreen window");
    m_visible = false;
    m_created = false;
}

} // namespace AsTestTool
