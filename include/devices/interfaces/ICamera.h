#pragma once

#include "IDevice.h"
#include <vector>
#include <string>
#include <cstdint>

namespace AsTestTool {

/**
 * @brief 分辨率结构
 */
struct Resolution {
    int width;
    int height;
    int fps;
    
    Resolution(int w = 0, int h = 0, int f = 30) : width(w), height(h), fps(f) {}
    
    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height && fps == other.fps;
    }
    
    std::string ToString() const {
        return std::to_string(width) + "x" + std::to_string(height) + "@" + std::to_string(fps) + "fps";
    }
};

/**
 * @brief 摄像头信息结构
 */
struct CameraInfo {
    std::string name;
    std::string devicePath;
    std::vector<Resolution> supportedResolutions;
    bool isAvailable;
    
    CameraInfo() : isAvailable(false) {}
};

/**
 * @brief 图像数据结构
 */
struct ImageData {
    std::vector<uint8_t> data;
    int width;
    int height;
    int channels;
    std::string format; // "RGB", "BGR", "RGBA", etc.
    
    ImageData() : width(0), height(0), channels(0) {}
    
    size_t GetDataSize() const {
        return data.size();
    }
    
    bool IsValid() const {
        return !data.empty() && width > 0 && height > 0 && channels > 0;
    }
};

/**
 * @brief 摄像头接口
 */
class ICamera : public IDevice {
public:
    virtual ~ICamera() = default;

    /**
     * @brief 开始预览
     * @return true 开始成功，false 开始失败
     */
    virtual bool StartPreview() = 0;

    /**
     * @brief 停止预览
     * @return true 停止成功，false 停止失败
     */
    virtual bool StopPreview() = 0;

    /**
     * @brief 拍照
     * @param imageData 输出的图像数据
     * @return true 拍照成功，false 拍照失败
     */
    virtual bool CaptureImage(ImageData& imageData) = 0;

    /**
     * @brief 获取可用摄像头列表
     * @return 摄像头信息列表
     */
    virtual std::vector<CameraInfo> GetAvailableCameras() = 0;

    /**
     * @brief 设置分辨率
     * @param width 宽度
     * @param height 高度
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetResolution(int width, int height) = 0;

    /**
     * @brief 获取当前分辨率
     * @return 当前分辨率
     */
    virtual Resolution GetCurrentResolution() = 0;

    /**
     * @brief 获取预览图像数据（用于实时显示）
     * @param imageData 输出的图像数据
     * @return true 获取成功，false 获取失败
     */
    virtual bool GetPreviewData(ImageData& imageData) = 0;

    /**
     * @brief 设置亮度
     * @param brightness 亮度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetBrightness(int brightness) = 0;

    /**
     * @brief 设置对比度
     * @param contrast 对比度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetContrast(int contrast) = 0;
};

} // namespace AsTestTool
