#include "gui/TextureRenderer.h"
#include "utils/Logger.h"

// Windows 头文件（必须在 OpenGL 头文件之前包含以避免冲突）
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// OpenGL 头文件
#ifdef _WIN32
#include <GL/gl.h>
// GL_CLAMP_TO_EDGE 在旧版 Windows SDK OpenGL 头文件中可能未定义
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#elif __linux__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

namespace AsTestTool {

TextureRenderer::TextureRenderer() = default;

TextureRenderer::~TextureRenderer() {
    Shutdown();
}

bool TextureRenderer::Initialize() {
    if (m_initialized) {
        return true;
    }

    // 创建 OpenGL 纹理
    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        m_lastError = "Failed to generate OpenGL texture";
        LOG_ERROR("TextureRenderer: " + m_lastError);
        return false;
    }

    // 配置纹理参数
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 创建 ImGui 纹理 ID（将 OpenGL 纹理 ID 转换为指针形式）
    m_imTextureID = reinterpret_cast<void*>(static_cast<size_t>(m_textureId));

    m_initialized = true;
    LOG_INFO("TextureRenderer: Initialized successfully");
    return true;
}

void TextureRenderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    DestroyTexture();
    m_convertBuffer.clear();
    m_initialized = false;
    
    LOG_INFO("TextureRenderer: Shutdown complete");
}

void TextureRenderer::DestroyTexture() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        m_imTextureID = nullptr;
        m_width = 0;
        m_height = 0;
    }
}

bool TextureRenderer::UploadBGR(const uint8_t* data, int width, int height) {
    // BGR -> RGBA 转换
    m_convertBuffer.resize(width * height * 4);
    
    for (int i = 0; i < width * height; i++) {
        m_convertBuffer[i * 4 + 0] = data[i * 3 + 2]; // R = B
        m_convertBuffer[i * 4 + 1] = data[i * 3 + 1]; // G = G
        m_convertBuffer[i * 4 + 2] = data[i * 3 + 0]; // B = R
        m_convertBuffer[i * 4 + 3] = 255;             // A = 255
    }
    
    return UploadImage(m_convertBuffer.data(), width, height, 4);
}

bool TextureRenderer::UploadRGB(const uint8_t* data, int width, int height) {
    return UploadImage(data, width, height, 3);
}

bool TextureRenderer::UploadRGBA(const uint8_t* data, int width, int height) {
    return UploadImage(data, width, height, 4);
}

bool TextureRenderer::UploadImage(const uint8_t* data, int width, int height, int channels) {
    if (!m_initialized) {
        if (!Initialize()) {
            return false;
        }
    }

    if (!data || width <= 0 || height <= 0) {
        m_lastError = "Invalid image parameters";
        return false;
    }

    return UpdateTextureData(data, width, height, channels);
}

bool TextureRenderer::UpdateTextureData(const uint8_t* data, int width, int height, int channels) {
    GLenum format;
    GLenum internalFormat;

    switch (channels) {
        case 1:
            format = GL_LUMINANCE;
            internalFormat = GL_LUMINANCE;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA;
            break;
        default:
            m_lastError = "Unsupported channel count: " + std::to_string(channels);
            LOG_ERROR("TextureRenderer: " + m_lastError);
            return false;
    }

    // 如果尺寸变了，重新创建纹理
    if (m_width != width || m_height != height) {
        DestroyTexture();
        glGenTextures(1, &m_textureId);
        if (m_textureId == 0) {
            m_lastError = "Failed to generate OpenGL texture";
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_imTextureID = reinterpret_cast<void*>(static_cast<size_t>(m_textureId));
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // 检查 OpenGL 错误
    GLenum error = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);

    if (error != GL_NO_ERROR) {
        m_lastError = "OpenGL error: " + std::to_string(error);
        LOG_ERROR("TextureRenderer: " + m_lastError);
        return false;
    }

    m_width = width;
    m_height = height;
    return true;
}

void TextureRenderer::Render(const ImVec2& pos, const ImVec2& size) {
    if (!HasValidTexture()) {
        return;
    }

    // 注意：实际渲染应该使用 ImGui::Image()，这里只提供纹理 ID
    // 调用者负责在 ImGui 的 DrawList 中添加图像绘制
    // 示例：
    // ImGui::SetCursorPos(pos);
    // ImGui::Image(m_textureRenderer.GetImTextureID(), size);
}

void TextureRenderer::Render(const ImVec2& pos) {
    if (!HasValidTexture()) {
        return;
    }
    Render(pos, ImVec2(static_cast<float>(m_width), static_cast<float>(m_height)));
}

ImTextureID TextureRenderer::GetImTextureID() const {
    return reinterpret_cast<ImTextureID>(m_imTextureID);
}

void TextureRenderer::Clear() {
    DestroyTexture();
    
    // 重新创建空纹理
    if (m_initialized) {
        glGenTextures(1, &m_textureId);
        if (m_textureId != 0) {
            glBindTexture(GL_TEXTURE_2D, m_textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_imTextureID = reinterpret_cast<void*>(static_cast<size_t>(m_textureId));
        }
    }
}

} // namespace AsTestTool
