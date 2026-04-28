#pragma once

#include "IDevice.h"
#include "ImageData.h"
#include <vector>
#include <string>

namespace AsTestTool {

/**
 * @brief 预览控制器接口
 * 
 * 分离预览相关的功能，提供更清晰的接口设计。
 */
class IPreviewController {
public:
    virtual ~IPreviewController() = default;

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
     * @brief 检查预览是否正在运行
     * @return true 正在预览，false 未预览
     */
    virtual bool IsPreviewActive() const = 0;

    /**
     * @brief 获取预览帧数据
     * @param data 输出的图像数据
     * @return true 获取成功，false 获取失败
     */
    virtual bool GetPreviewFrame(ImageData& data) = 0;
};

} // namespace AsTestTool
