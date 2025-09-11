#pragma once

#include "IDevice.h"
#include <vector>
#include <string>
#include <cstdint>

namespace AsTestTool {

/**
 * @brief 手写点结构
 */
struct SignaturePoint {
    float x, y;           // 坐标
    float pressure;       // 压力值 (0.0-1.0)
    uint64_t timestamp;   // 时间戳
    bool isDown;          // 是否按下
    
    SignaturePoint(float x = 0.0f, float y = 0.0f, float p = 0.0f, 
                   uint64_t ts = 0, bool down = false)
        : x(x), y(y), pressure(p), timestamp(ts), isDown(down) {}
};

/**
 * @brief 手写数据结构
 */
struct SignatureData {
    std::vector<SignaturePoint> points;
    int width, height;    // 手写屏尺寸
    std::string deviceInfo;
    
    SignatureData() : width(0), height(0) {}
    
    /**
     * @brief 清空手写数据
     */
    void Clear() {
        points.clear();
        width = 0;
        height = 0;
        deviceInfo.clear();
    }
    
    /**
     * @brief 检查是否有手写数据
     * @return true 有数据，false 无数据
     */
    bool HasData() const {
        return !points.empty();
    }
    
    /**
     * @brief 获取手写轨迹长度
     * @return 轨迹点数量
     */
    size_t GetPointCount() const {
        return points.size();
    }
};

/**
 * @brief 手写屏接口
 */
class ISignaturePad : public IDevice {
public:
    virtual ~ISignaturePad() = default;

    /**
     * @brief 开始捕获手写数据
     * @return true 开始成功，false 开始失败
     */
    virtual bool StartCapture() = 0;

    /**
     * @brief 停止捕获手写数据
     * @return true 停止成功，false 停止失败
     */
    virtual bool StopCapture() = 0;

    /**
     * @brief 获取手写数据
     * @param data 输出的手写数据
     * @return true 获取成功，false 获取失败
     */
    virtual bool GetSignatureData(SignatureData& data) = 0;

    /**
     * @brief 清除手写数据
     * @return true 清除成功，false 清除失败
     */
    virtual bool ClearSignature() = 0;

    /**
     * @brief 获取手写屏尺寸
     * @param width 输出宽度
     * @param height 输出高度
     * @return true 获取成功，false 获取失败
     */
    virtual bool GetScreenSize(int& width, int& height) = 0;

    /**
     * @brief 设置手写屏全屏模式
     * @param fullscreen true 全屏，false 窗口模式
     * @return true 设置成功，false 设置失败
     */
    virtual bool SetFullscreen(bool fullscreen) = 0;

    /**
     * @brief 检查是否正在捕获
     * @return true 正在捕获，false 未捕获
     */
    virtual bool IsCapturing() = 0;

    /**
     * @brief 获取支持的手写屏列表
     * @return 手写屏名称列表
     */
    virtual std::vector<std::string> GetSupportedPads() = 0;
};

} // namespace AsTestTool
