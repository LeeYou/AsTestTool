#pragma once

#include "IDevice.h"
#include <vector>
#include <string>

namespace AsTestTool {

/**
 * @brief 身份证信息结构
 */
struct IDCardInfo {
    std::string name;           // 姓名
    std::string gender;         // 性别
    std::string nation;         // 民族
    std::string birthDate;      // 出生日期
    std::string address;        // 住址
    std::string idNumber;       // 身份证号
    std::string issuingAuthority; // 签发机关
    std::string validPeriod;    // 有效期限
    std::vector<uint8_t> photo; // 照片数据
    
    IDCardInfo() = default;
    
    /**
     * @brief 检查身份证信息是否有效
     * @return true 信息有效，false 信息无效
     */
    bool IsValid() const {
        return !name.empty() && !idNumber.empty();
    }
    
    /**
     * @brief 清空身份证信息
     */
    void Clear() {
        name.clear();
        gender.clear();
        nation.clear();
        birthDate.clear();
        address.clear();
        idNumber.clear();
        issuingAuthority.clear();
        validPeriod.clear();
        photo.clear();
    }
};

/**
 * @brief 身份证阅读器接口
 */
class IIDCardReader : public IDevice {
public:
    virtual ~IIDCardReader() = default;

    /**
     * @brief 读取身份证信息
     * @param info 输出的身份证信息
     * @return true 读取成功，false 读取失败
     */
    virtual bool ReadCard(IDCardInfo& info) = 0;

    /**
     * @brief 获取支持的阅读器列表
     * @return 阅读器名称列表
     */
    virtual std::vector<std::string> GetSupportedReaders() = 0;

    /**
     * @brief 检查是否有身份证插入
     * @return true 有身份证，false 无身份证
     */
    virtual bool HasCard() = 0;

    /**
     * @brief 弹出身份证
     * @return true 弹出成功，false 弹出失败
     */
    virtual bool EjectCard() = 0;
};

} // namespace AsTestTool
