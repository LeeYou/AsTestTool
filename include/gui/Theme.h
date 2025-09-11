#pragma once

namespace AsTestTool {

/**
 * @brief 主题管理类
 */
class Theme {
public:
    /**
     * @brief 获取单例实例
     * @return Theme实例
     */
    static Theme& Instance();

    /**
     * @brief 应用亮色主题
     */
    void ApplyLightTheme();

    /**
     * @brief 应用暗色主题
     */
    void ApplyDarkTheme();

    /**
     * @brief 设置自定义主题
     */
    void SetCustomTheme();

private:
    Theme() = default;
    ~Theme() = default;
    Theme(const Theme&) = delete;
    Theme& operator=(const Theme&) = delete;
};

} // namespace AsTestTool
