#pragma once

#include <string>
#include <vector>

namespace AsTestTool {

/**
 * @brief 字符串工具类
 */
class StringUtils {
public:
    /**
     * @brief 去除字符串前后空格
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    static std::string Trim(const std::string& str);

    /**
     * @brief 去除字符串左侧空格
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    static std::string TrimLeft(const std::string& str);

    /**
     * @brief 去除字符串右侧空格
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    static std::string TrimRight(const std::string& str);

    /**
     * @brief 转换为小写
     * @param str 输入字符串
     * @return 小写字符串
     */
    static std::string ToLower(const std::string& str);

    /**
     * @brief 转换为大写
     * @param str 输入字符串
     * @return 大写字符串
     */
    static std::string ToUpper(const std::string& str);

    /**
     * @brief 分割字符串
     * @param str 输入字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串列表
     */
    static std::vector<std::string> Split(const std::string& str, const std::string& delimiter);

    /**
     * @brief 连接字符串
     * @param strings 字符串列表
     * @param delimiter 分隔符
     * @return 连接后的字符串
     */
    static std::string Join(const std::vector<std::string>& strings, const std::string& delimiter);

    /**
     * @brief 替换字符串中的子串
     * @param str 输入字符串
     * @param from 要替换的子串
     * @param to 替换为的子串
     * @return 替换后的字符串
     */
    static std::string Replace(const std::string& str, const std::string& from, const std::string& to);

    /**
     * @brief 检查字符串是否以指定前缀开始
     * @param str 输入字符串
     * @param prefix 前缀
     * @return true 以指定前缀开始，false 不是
     */
    static bool StartsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief 检查字符串是否以指定后缀结束
     * @param str 输入字符串
     * @param suffix 后缀
     * @return true 以指定后缀结束，false 不是
     */
    static bool EndsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief 检查字符串是否包含指定子串
     * @param str 输入字符串
     * @param substr 子串
     * @return true 包含，false 不包含
     */
    static bool Contains(const std::string& str, const std::string& substr);
};

} // namespace AsTestTool
