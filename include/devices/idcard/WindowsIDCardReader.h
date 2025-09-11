#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include <string>

namespace AsTestTool {

/**
 * @brief Windows平台身份证阅读器实现
 */
class WindowsIDCardReader : public IIDCardReader {
public:
    explicit WindowsIDCardReader(const std::string& type = "");
    virtual ~WindowsIDCardReader();

    // IDevice接口实现
    bool Initialize() override;
    bool IsConnected() override;
    void Shutdown() override;
    std::string GetDeviceInfo() override;

    // IIDCardReader接口实现
    bool ReadCard(IDCardInfo& info) override;
    std::vector<std::string> GetSupportedReaders() override;
    bool HasCard() override;
    bool EjectCard() override;

private:
    std::string m_type;
    bool m_initialized = false;
};

} // namespace AsTestTool
