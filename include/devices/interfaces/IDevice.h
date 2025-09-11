#pragma once

#include <string>

namespace AsTestTool {

/**
 * @brief 基础设备接口
 */
class IDevice {
public:
    virtual ~IDevice() = default;

    /**
     * @brief 初始化设备
     * @return true 初始化成功，false 初始化失败
     */
    virtual bool Initialize() = 0;

    /**
     * @brief 检查设备是否已连接
     * @return true 设备已连接，false 设备未连接
     */
    virtual bool IsConnected() = 0;

    /**
     * @brief 关闭设备
     */
    virtual void Shutdown() = 0;

    /**
     * @brief 获取设备信息
     * @return 设备信息字符串
     */
    virtual std::string GetDeviceInfo() = 0;
};

} // namespace AsTestTool
