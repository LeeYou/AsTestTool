#pragma once

#include "devices/interfaces/IDevice.h"
#include "devices/interfaces/IIDCardReader.h"
#include "devices/interfaces/ICamera.h"
#include "devices/interfaces/ISignaturePad.h"
#include "utils/Singleton.h"
#include <memory>
#include <vector>

namespace AsTestTool {

/**
 * @brief 设备管理器类
 * 
 * 统一管理所有设备实例，负责：
 * - 设备的创建和销毁
 * - 设备的初始化和关闭
 * - 设备的生命周期管理
 * - 设备状态查询
 */
class DeviceManager : public Singleton<DeviceManager> {
    friend class Singleton<DeviceManager>;

public:
    /**
     * @brief 初始化所有设备
     * @return true 初始化成功，false 初始化失败
     */
    bool InitializeAll();

    /**
     * @brief 关闭所有设备
     */
    void ShutdownAll();

    /**
     * @brief 获取身份证阅读器
     * @return 身份证阅读器指针，未初始化返回 nullptr
     */
    IIDCardReader* GetIDCardReader() const { return m_idCardReader.get(); }

    /**
     * @brief 获取摄像头
     * @return 摄像头指针，未初始化返回 nullptr
     */
    ICamera* GetCamera() const { return m_camera.get(); }

    /**
     * @brief 获取手写屏
     * @return 手写屏指针，未初始化返回 nullptr
     */
    ISignaturePad* GetSignaturePad() const { return m_signaturePad.get(); }

    /**
     * @brief 检查身份证阅读器是否已连接
     */
    bool IsIDCardReaderConnected() const;

    /**
     * @brief 检查摄像头是否已连接
     */
    bool IsCameraConnected() const;

    /**
     * @brief 检查手写屏是否已连接
     */
    bool IsSignaturePadConnected() const;

    /**
     * @brief 获取设备状态摘要
     */
    struct DeviceStatus {
        bool idCardConnected = false;
        bool cameraConnected = false;
        bool signatureConnected = false;
    };
    DeviceStatus GetStatus() const;

    /**
     * @brief 获取设备信息摘要
     */
    std::string GetStatusSummary() const;

private:
    DeviceManager() = default;
    ~DeviceManager() = default;

    // 禁止拷贝和赋值
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    // 私有初始化方法
    bool InitializeIDCardReader();
    bool InitializeCamera();
    bool InitializeSignaturePad();

    // 私有关闭方法
    void ShutdownIDCardReader();
    void ShutdownCamera();
    void ShutdownSignaturePad();

private:
    std::unique_ptr<IIDCardReader> m_idCardReader;
    std::unique_ptr<ICamera> m_camera;
    std::unique_ptr<ISignaturePad> m_signaturePad;
    bool m_initialized = false;
};

} // namespace AsTestTool
