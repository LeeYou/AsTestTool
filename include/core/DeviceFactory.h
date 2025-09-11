#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include "devices/interfaces/ICamera.h"
#include "devices/interfaces/ISignaturePad.h"
#include <memory>
#include <string>

namespace AsTestTool {

/**
 * @brief 设备工厂类
 */
class DeviceFactory {
public:
    /**
     * @brief 创建身份证阅读器
     * @param type 阅读器类型（可选，为空时使用默认）
     * @return 身份证阅读器实例
     */
    static std::unique_ptr<IIDCardReader> CreateIDCardReader(const std::string& type = "");

    /**
     * @brief 创建摄像头
     * @return 摄像头实例
     */
    static std::unique_ptr<ICamera> CreateCamera();

    /**
     * @brief 创建手写屏
     * @param type 手写屏类型（可选，为空时使用默认）
     * @return 手写屏实例
     */
    static std::unique_ptr<ISignaturePad> CreateSignaturePad(const std::string& type = "");

    /**
     * @brief 获取支持的身份证阅读器类型列表
     * @return 阅读器类型列表
     */
    static std::vector<std::string> GetSupportedIDCardReaders();

    /**
     * @brief 获取支持的手写屏类型列表
     * @return 手写屏类型列表
     */
    static std::vector<std::string> GetSupportedSignaturePads();

private:
    DeviceFactory() = default;
    ~DeviceFactory() = default;
    DeviceFactory(const DeviceFactory&) = delete;
    DeviceFactory& operator=(const DeviceFactory&) = delete;
};

} // namespace AsTestTool
