#pragma once

#include <string>
#include <vector>

namespace AsTestTool {

/**
 * @brief 文件工具类
 */
class FileUtils {
public:
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return true 存在，false 不存在
     */
    static bool FileExists(const std::string& path);

    /**
     * @brief 检查目录是否存在
     * @param path 目录路径
     * @return true 存在，false 不存在
     */
    static bool DirectoryExists(const std::string& path);

    /**
     * @brief 创建目录
     * @param path 目录路径
     * @return true 创建成功，false 创建失败
     */
    static bool CreateDirectory(const std::string& path);

    /**
     * @brief 获取文件大小
     * @param path 文件路径
     * @return 文件大小（字节），-1表示失败
     */
    static long long GetFileSize(const std::string& path);

    /**
     * @brief 读取文件内容
     * @param path 文件路径
     * @param content 输出的文件内容
     * @return true 读取成功，false 读取失败
     */
    static bool ReadFile(const std::string& path, std::string& content);

    /**
     * @brief 写入文件内容
     * @param path 文件路径
     * @param content 文件内容
     * @return true 写入成功，false 写入失败
     */
    static bool WriteFile(const std::string& path, const std::string& content);

    /**
     * @brief 获取文件扩展名
     * @param path 文件路径
     * @return 扩展名（包含点号）
     */
    static std::string GetFileExtension(const std::string& path);

    /**
     * @brief 获取文件名（不包含路径）
     * @param path 文件路径
     * @return 文件名
     */
    static std::string GetFileName(const std::string& path);

    /**
     * @brief 获取目录路径
     * @param path 文件路径
     * @return 目录路径
     */
    static std::string GetDirectoryPath(const std::string& path);

    /**
     * @brief 列出目录中的文件
     * @param path 目录路径
     * @param files 输出的文件列表
     * @return true 成功，false 失败
     */
    static bool ListFiles(const std::string& path, std::vector<std::string>& files);
};

} // namespace AsTestTool
