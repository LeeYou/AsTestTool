#pragma once

#include <string>

namespace AsTestTool {

/**
 * @brief 图像设置接口
 * 
 * 分离图像参数设置相关的功能，提供更清晰的接口设计。
 */
class IImageSettings {
public:
    virtual ~IImageSettings() = default;

    /**
     * @brief 设置分辨率
     * @param width 宽度
     * @param height 高度
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetResolution(int width, int height) = 0;

    /**
     * @brief 获取当前分辨率
     * @param width 输出宽度
     * @param height 输出高度
     */
    virtual void GetResolution(int& width, int& height) const = 0;

    /**
     * @brief 设置亮度
     * @param value 亮度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetBrightness(int value) = 0;

    /**
     * @brief 获取亮度
     * @return 当前亮度值 (0-100)
     */
    virtual int GetBrightness() const = 0;

    /**
     * @brief 设置对比度
     * @param value 对比度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetContrast(int value) = 0;

    /**
     * @brief 获取对比度
     * @return 当前对比度值 (0-100)
     */
    virtual int GetContrast() const = 0;

    /**
     * @brief 设置饱和度
     * @param value 饱和度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetSaturation(int value) = 0;

    /**
     * @brief 获取饱和度
     * @return 当前饱和度值 (0-100)
     */
    virtual int GetSaturation() const = 0;

    /**
     * @brief 设置锐度
     * @param value 锐度值 (0-100)
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetSharpness(int value) = 0;

    /**
     * @brief 获取锐度
     * @return 当前锐度值 (0-100)
     */
    virtual int GetSharpness() const = 0;

    /**
     * @brief 重置所有设置到默认值
     */
    virtual void ResetToDefaults() = 0;
};

} // namespace AsTestTool
