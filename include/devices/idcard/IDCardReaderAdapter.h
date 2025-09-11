#pragma once

#include "devices/interfaces/IIDCardReader.h"
#include "devices/idcard/LibraryLoader.h"
#include <string>
#include <memory>

namespace AsTestTool {

/**
 * @brief 身份证阅读器适配器类
 */
class IDCardReaderAdapter : public IIDCardReader {
public:
    explicit IDCardReaderAdapter(const std::string& libraryPath);
    virtual ~IDCardReaderAdapter();

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
    std::string m_libraryPath;
    std::unique_ptr<LibraryLoader> m_loader;
    bool m_initialized = false;
};

} // namespace AsTestTool
