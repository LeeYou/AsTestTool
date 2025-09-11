#pragma once

#include <string>
#include <map>
#include <vector>

namespace AsTestTool {

/**
 * @brief 配置管理类
 */
class Config {
public:
    /**
     * @brief 获取单例实例
     * @return Config实例
     */
    static Config& Instance();

    /**
     * @brief 加载配置文件
     * @param filename 配置文件路径
     * @return true 加载成功，false 加载失败
     */
    bool LoadFromFile(const std::string& filename);

    /**
     * @brief 保存配置文件
     * @param filename 配置文件路径
     * @return true 保存成功，false 保存失败
     */
    bool SaveToFile(const std::string& filename);

    /**
     * @brief 获取字符串配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;

    /**
     * @brief 设置字符串配置
     * @param key 配置键
     * @param value 配置值
     */
    void SetString(const std::string& key, const std::string& value);

    /**
     * @brief 获取整数配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    int GetInt(const std::string& key, int defaultValue = 0) const;

    /**
     * @brief 设置整数配置
     * @param key 配置键
     * @param value 配置值
     */
    void SetInt(const std::string& key, int value);

    /**
     * @brief 获取布尔配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    bool GetBool(const std::string& key, bool defaultValue = false) const;

    /**
     * @brief 设置布尔配置
     * @param key 配置键
     * @param value 配置值
     */
    void SetBool(const std::string& key, bool value);

    /**
     * @brief 获取浮点数配置
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    double GetDouble(const std::string& key, double defaultValue = 0.0) const;

    /**
     * @brief 设置浮点数配置
     * @param key 配置键
     * @param value 配置值
     */
    void SetDouble(const std::string& key, double value);

    /**
     * @brief 检查配置是否存在
     * @param key 配置键
     * @return true 存在，false 不存在
     */
    bool HasKey(const std::string& key) const;

    /**
     * @brief 删除配置
     * @param key 配置键
     */
    void RemoveKey(const std::string& key);

    /**
     * @brief 清空所有配置
     */
    void Clear();

    /**
     * @brief 获取所有配置键
     * @return 配置键列表
     */
    std::vector<std::string> GetAllKeys() const;

    /**
     * @brief 设置默认配置
     */
    void SetDefaults();

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::string> m_config;
};

} // namespace AsTestTool
