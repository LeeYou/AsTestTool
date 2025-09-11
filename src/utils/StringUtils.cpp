#include "utils/StringUtils.h"
#include <algorithm>
#include <sstream>

namespace AsTestTool {

std::string StringUtils::Trim(const std::string& str) {
    return TrimLeft(TrimRight(str));
}

std::string StringUtils::TrimLeft(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string StringUtils::TrimRight(const std::string& str) {
    size_t end = str.find_last_not_of(" \t\n\r");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string StringUtils::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> StringUtils::Split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    if (delimiter.empty()) {
        result.push_back(str);
        return result;
    }

    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    result.push_back(str.substr(start));
    return result;
}

std::string StringUtils::Join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }

    std::ostringstream oss;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            oss << delimiter;
        }
        oss << strings[i];
    }
    return oss.str();
}

std::string StringUtils::Replace(const std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return str;
    }

    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

bool StringUtils::StartsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    return str.substr(0, prefix.length()) == prefix;
}

bool StringUtils::EndsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool StringUtils::Contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

} // namespace AsTestTool
