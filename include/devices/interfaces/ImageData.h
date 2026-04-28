#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace AsTestTool {

/**
 * @brief 图像数据结构
 */
struct ImageData {
    std::vector<uint8_t> data;  // 图像数据
    int width = 0;              // 宽度
    int height = 0;             // 高度
    int channels = 0;           // 通道数
    std::string format;         // 格式描述，如 "RGB", "BGR", "RGBA"

    ImageData() = default;

    /**
     * @brief 获取数据大小
     * @return 数据字节数
     */
    size_t GetDataSize() const {
        return data.size();
    }

    /**
     * @brief 检查数据是否有效
     * @return true 有效，false 无效
     */
    bool IsValid() const {
        return !data.empty() && width > 0 && height > 0 && channels > 0;
    }

    /**
     * @brief 清空数据
     */
    void Clear() {
        data.clear();
        width = 0;
        height = 0;
        channels = 0;
        format.clear();
    }

    /**
     * @brief 获取格式描述字符串
     * @return 格式字符串
     */
    std::string GetFormatString() const {
        return format.empty() ? "Unknown" : format;
    }
};

/**
 * @brief 分辨率结构
 */
struct Resolution {
    int width = 0;
    int height = 0;
    int fps = 30;
    std::string description;

    Resolution() = default;

    Resolution(int w, int h, int f = 30, const std::string& desc = "")
        : width(w), height(h), fps(f), description(desc) {}

    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height && fps == other.fps;
    }

    std::string ToString() const {
        if (!description.empty()) {
            return description + " (" + std::to_string(width) + "x" + 
                   std::to_string(height) + "@" + std::to_string(fps) + ")";
        }
        return std::to_string(width) + "x" + std::to_string(height) + 
               "@" + std::to_string(fps);
    }

    float GetAspectRatio() const {
        if (height == 0) return 1.0f;
        return static_cast<float>(width) / height;
    }
};

/**
 * @brief 摄像头信息结构
 */
struct CameraInfo {
    std::string id;                                      // 设备 ID
    std::string name;                                    // 设备名称
    std::string devicePath;                              // 设备路径
    std::vector<Resolution> supportedResolutions;        // 支持的分辨率
    bool isAvailable = false;                           // 是否可用

    CameraInfo() = default;

    bool HasResolution(int width, int height) const {
        for (const auto& res : supportedResolutions) {
            if (res.width == width && res.height == height) {
                return true;
            }
        }
        return false;
    }
};

} // namespace AsTestTool
