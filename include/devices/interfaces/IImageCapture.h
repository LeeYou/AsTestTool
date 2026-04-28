#pragma once

#include "ImageData.h"
#include <vector>
#include <string>

namespace AsTestTool {

/**
 * @brief 像素格式枚举
 */
enum class PixelFormat {
    YUV420,
    YUV422,
    RGB24,
    RGB32,
    MJPG,
    H264,
    Unknown
};

/**
 * @brief 图像捕获器接口
 * 
 * 分离图像捕获相关的功能，提供更清晰的接口设计。
 */
class IImageCapture {
public:
    virtual ~IImageCapture() = default;

    /**
     * @brief 捕获图像
     * @param data 输出的图像数据
     * @return true 捕获成功，false 捕获失败
     */
    virtual bool CaptureImage(ImageData& data) = 0;

    /**
     * @brief 设置捕获格式
     * @param format 像素格式
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetCaptureFormat(PixelFormat format) = 0;

    /**
     * @brief 获取当前捕获格式
     * @return 当前像素格式
     */
    virtual PixelFormat GetCaptureFormat() const = 0;

    /**
     * @brief 获取最后捕获的图像
     * @param data 输出的图像数据
     * @return true 获取成功，false 获取失败
     */
    virtual bool GetLastCapturedImage(ImageData& data) = 0;
};

} // namespace AsTestTool
