#pragma once

#include <cstdint>
#include <string>
#include <vector>

// ImGui 头文件（必须在 OpenGL 头文件之前包含）
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <imgui.h>

namespace AsTestTool {

/**
 * @brief 纹理渲染器类
 * 
 * 使用 OpenGL 纹理实现高效的图像渲染，相比逐像素绘制性能提升显著。
 * 支持：
 * - 纹理上传（RGB, RGBA, BGR 格式）
 * - 自动格式转换
 * - 纹理缓存
 * - 缩放渲染
 */
class TextureRenderer {
public:
    TextureRenderer();
    ~TextureRenderer();

    // 禁止拷贝
    TextureRenderer(const TextureRenderer&) = delete;
    TextureRenderer& operator=(const TextureRenderer&) = delete;

    /**
     * @brief 初始化渲染器
     * @return true 初始化成功，false 初始化失败
     */
    bool Initialize();

    /**
     * @brief 关闭渲染器
     */
    void Shutdown();

    /**
     * @brief 检查是否已初始化
     */
    bool IsInitialized() const { return m_initialized; }

    /**
     * @brief 上传图像数据到纹理
     * 
     * @param data 图像数据指针
     * @param width 图像宽度
     * @param height 图像高度
     * @param channels 通道数 (3=RGB, 4=RGBA, 支持1=BGR自动转换)
     * @return true 上传成功，false 上传失败
     */
    bool UploadImage(const uint8_t* data, int width, int height, int channels);

    /**
     * @brief 上传 BGR 格式图像（用于摄像头数据）
     */
    bool UploadBGR(const uint8_t* data, int width, int height);

    /**
     * @brief 上传 RGB 格式图像
     */
    bool UploadRGB(const uint8_t* data, int width, int height);

    /**
     * @brief 上传 RGBA 格式图像
     */
    bool UploadRGBA(const uint8_t* data, int width, int height);

    /**
     * @brief 渲染纹理到指定位置
     * 
     * @param pos 左上角位置
     * @param size 渲染尺寸
     */
    void Render(const ImVec2& pos, const ImVec2& size);

    /**
     * @brief 渲染纹理（使用原始尺寸）
     */
    void Render(const ImVec2& pos);

    /**
     * @brief 获取 ImGui 纹理 ID（用于 ImGui::Image）
     */
    ImTextureID GetImTextureID() const;

    /**
     * @brief 获取当前纹理宽度
     */
    int GetWidth() const { return m_width; }

    /**
     * @brief 获取当前纹理高度
     */
    int GetHeight() const { return m_height; }

    /**
     * @brief 检查是否有有效纹理
     */
    bool HasValidTexture() const { return m_textureId != 0 && m_width > 0 && m_height > 0; }

    /**
     * @brief 清除纹理内容
     */
    void Clear();

    /**
     * @brief 获取最后错误信息
     */
    const std::string& GetLastError() const { return m_lastError; }

private:
    /**
     * @brief 创建 OpenGL 纹理
     */
    bool CreateTexture();

    /**
     * @brief 销毁 OpenGL 纹理
     */
    void DestroyTexture();

    /**
     * @brief 更新 OpenGL 纹理数据
     */
    bool UpdateTextureData(const uint8_t* data, int width, int height, int channels);

private:
    bool m_initialized = false;
    
    // OpenGL 纹理 ID
    unsigned int m_textureId = 0;
    
    // 当前纹理尺寸
    int m_width = 0;
    int m_height = 0;
    
    // ImGui 纹理 ID（存储 OpenGL 纹理 ID 的指针形式）
    void* m_imTextureID = nullptr;
    
    // 错误信息
    std::string m_lastError;

    // 临时缓冲区（用于格式转换）
    std::vector<uint8_t> m_convertBuffer;
};

} // namespace AsTestTool
