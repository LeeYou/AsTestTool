#include "gui/IDCardPanel.h"
#include "gui/PanelUiHelpers.h"
#include "core/DeviceFactory.h"
#include "utils/Logger.h"
#include "utils/FileDialog.h"
#include "core/DeviceManager.h"
#include "imgui.h"
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include "devices/idcard/WindowsIDCardReader.h"
#endif

namespace AsTestTool {

namespace {

bool OpenPanelIDCardDevice(IIDCardReader* reader, int port) {
    if (!reader) {
        return false;
    }

#ifdef PLATFORM_WINDOWS
    if (auto* windowsReader = dynamic_cast<WindowsIDCardReader*>(reader)) {
        return windowsReader->OpenDevice(port);
    }
#endif

    return true;
}

bool LoadPanelIDCardLibrary(IIDCardReader* reader, const std::string& libraryPath) {
#ifdef PLATFORM_WINDOWS
    if (auto* windowsReader = dynamic_cast<WindowsIDCardReader*>(reader)) {
        return windowsReader->LoadLibrary(libraryPath);
    }
#endif

    LOG_ERROR("Custom ID card library reload is not supported on this platform");
    return false;
}

}

IDCardPanel::IDCardPanel() {
    LOG_INFO("IDCardPanel created");
}

IDCardPanel::~IDCardPanel() {
    LOG_INFO("IDCardPanel destroyed");
}

void IDCardPanel::SetIDCardReader(IIDCardReader* reader) {
    m_customReader.reset();
    m_idCardReader = reader;
    LOG_INFO("ID card reader set to panel");
}

void IDCardPanel::Render() {
    RenderDeviceStatus();
    RenderControls();
    RenderCardInfo();
    
    if (m_showLibrarySettings) {
        RenderLibrarySettings();
    }
}

void IDCardPanel::RenderDeviceStatus() {
    std::string statusText = GetDeviceStatusText();
    ImVec4 statusColor = GetDeviceStatusColor();
    const std::string deviceInfo = m_idCardReader ? m_idCardReader->GetDeviceInfo() : std::string();
    PanelUi::RenderDeviceStatusBlock(statusText.c_str(), statusColor, deviceInfo.c_str());
}

void IDCardPanel::RenderCardInfo() {
    PanelUi::RenderSectionTitle("身份证信息:");
    bool hasValidCardInfo = m_hasCardInfo && m_currentCardInfo.IsValid();
    ImVec4 infoStatusColor = hasValidCardInfo ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    PanelUi::RenderStatusSummary("读取状态:", hasValidCardInfo ? "已读取" : "未读取", infoStatusColor);
    
    if (hasValidCardInfo) {
        PanelUi::RenderStateBanner("=== 身份证信息已读取 ===", infoStatusColor);
        PanelUi::RenderResponsiveInfoFields(
            "IDCardInfoColumns",
            std::array<PanelUi::InfoField, 6>{{
                {"姓名:", m_currentCardInfo.name.c_str()},
                {"性别:", m_currentCardInfo.gender.c_str()},
                {"民族:", m_currentCardInfo.nation.c_str()},
                {"出生日期:", m_currentCardInfo.birthDate.c_str()},
                {"身份证号:", m_currentCardInfo.idNumber.c_str()},
                {"有效期限:", m_currentCardInfo.validPeriod.c_str()}
            }},
            std::array<PanelUi::InfoField, 2>{{
                {"签发机关:", m_currentCardInfo.issuingAuthority.c_str()},
                {"住址:", m_currentCardInfo.address.c_str()}
            }}
        );
        
        PanelUi::RenderSectionTitle("照片信息:");
        if (!m_currentCardInfo.photo.empty()) {
            ImGui::Text("照片: 已获取 (BASE64编码)");
            ImGui::Text("照片大小: %zu 字节", m_currentCardInfo.photo.size());
        } else {
            ImGui::Text("照片: 无");
        }
    } else {
        PanelUi::RenderStateBanner("=== 身份证信息未读取 ===", infoStatusColor);
        PanelUi::RenderResponsiveInfoFields(
            "IDCardInfoColumnsEmpty",
            std::array<PanelUi::InfoField, 6>{{
                {"姓名:", "未读取"},
                {"性别:", "未读取"},
                {"民族:", "未读取"},
                {"出生日期:", "未读取"},
                {"身份证号:", "未读取"},
                {"有效期限:", "未读取"}
            }},
            std::array<PanelUi::InfoField, 2>{{
                {"签发机关:", "未读取"},
                {"住址:", "未读取"}
            }}
        );
        
        PanelUi::RenderSectionTitle("照片信息:");
        ImGui::Text("照片: 无");
    }
}

void IDCardPanel::RenderControls() {
    PanelUi::RenderSectionTitle("主要操作:");
    PanelUi::RenderConnectionToggleButton(
        IsDeviceConnected(),
        [this]() { ConnectDevice(); },
        [this]() { DisconnectDevice(); }
    );
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    
    // 读取身份证控制
    bool canRead = IsDeviceConnected() && m_idCardReader;
    PanelUi::RenderActionButton("读取身份证", [this]() { ReadCard(); }, !canRead);
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    
    // 弹出卡片控制
    bool canEject = IsDeviceConnected();
    PanelUi::RenderActionButton("弹出卡片", [this]() { EjectCard(); }, !canEject);
    
    PanelUi::RenderSectionTitle("库管理:");
    
    // 库设置按钮
    if (ImGui::Button("库设置", ImVec2(PanelUi::kButtonWidth, PanelUi::kButtonHeight))) {
        m_showLibrarySettings = !m_showLibrarySettings;
    }
    
    PanelUi::ContinueOnSameLineIfFits(PanelUi::kButtonWidth);
    
    // 重新加载库按钮
    PanelUi::RenderActionButton("重新加载库", [this]() {
        LoadIDCardLibrary();
    });
}

void IDCardPanel::RenderLibrarySettings() {
    PanelUi::PrepareLibrarySettingsWindow();
    if (ImGui::Begin("身份证阅读器库设置", &m_showLibrarySettings)) {
        PanelUi::RenderSectionTitle("基础配置:");
        
        char pathBuffer[512];
        strncpy_s(pathBuffer, m_libraryPath.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        
        if (PanelUi::RenderPathSelectorRow("库文件路径:", "##LibraryPath", pathBuffer, sizeof(pathBuffer), [this]() {
            auto fileDialog = FileDialog::Create();
            if (fileDialog) {
                std::vector<FileDialogFilter> filters = {
                    {"动态链接库 (*.dll)", "*.dll"},
                    {"所有文件 (*.*)", "*.*"}
                };
                auto result = fileDialog->OpenFile("选择身份证阅读器DLL文件", filters);
                if (result.success) {
                    m_libraryPath = result.filePath;
                    LOG_INFO("Selected DLL file: " + m_libraryPath);
                }
            }
        })) {
            m_libraryPath = std::string(pathBuffer);
        }
        
        PanelUi::BeginInlineFieldRow("设备端口:", 0.0f, 120.0f);
        ImGui::InputInt("##DevicePort", &m_devicePort, 1, 10);
        
        PanelUi::RenderDefaultLibrarySection("CMCC_IDCARD.DLL");
        
        PanelUi::RenderDescriptionSection({
            "- 库文件路径: 身份证阅读器DLL文件路径",
            "- 设备端口: USB端口从1000开始，USB1填1001",
            "- 支持的库: cmcc_idcard.dll 等标准接口库"
        });

        PanelUi::RenderSettingsActionSection(&m_showLibrarySettings, [this]() {
            LoadIDCardLibrary();
        });
    }
    ImGui::End();
}

void IDCardPanel::ConnectDevice() {
    LOG_INFO("Connecting ID card device");
    
    if (m_customReader) {
        m_idCardReader = m_customReader.get();
    } else {
        m_idCardReader = DeviceManager::Instance().GetIDCardReader();
    }
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    if (!m_idCardReader->Initialize()) {
        LOG_ERROR("Failed to connect ID card device");
        return;
    }

    if (!OpenPanelIDCardDevice(m_idCardReader, m_devicePort)) {
        LOG_ERROR("Failed to open ID card device on port: " + std::to_string(m_devicePort));
        return;
    }

    LOG_INFO("ID card device connected successfully");
}

void IDCardPanel::DisconnectDevice() {
    LOG_INFO("Disconnecting ID card device");
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    m_idCardReader->Shutdown();
    LOG_INFO("ID card device disconnected successfully");
    // 清空当前卡片信息
    m_hasCardInfo = false;
    m_currentCardInfo.Clear();
}

void IDCardPanel::ReadCard() {
    LOG_INFO("Reading ID card");
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    // 检查设备状态
    bool hasCard = m_idCardReader->HasCard();
    LOG_INFO("HasCard() returned: " + std::string(hasCard ? "true" : "false"));
    
    if (!hasCard) {
        LOG_WARNING("No card detected in reader - attempting to read anyway");
        // 注释掉这个检查，直接尝试读取
        // return;
    }
    
    IDCardInfo cardInfo;
    if (m_idCardReader->ReadCard(cardInfo)) {
        m_currentCardInfo = cardInfo;
        m_hasCardInfo = true;
        LOG_INFO("ID card read successfully");
        LOG_INFO("Name: " + cardInfo.name);
        LOG_INFO("ID Number: " + cardInfo.idNumber);
        LOG_INFO("Card info valid: " + std::string(cardInfo.IsValid() ? "true" : "false"));
        LOG_INFO("m_hasCardInfo set to: " + std::string(m_hasCardInfo ? "true" : "false"));
    } else {
        LOG_ERROR("Failed to read ID card");
        m_hasCardInfo = false;
        m_currentCardInfo.Clear();
    }
}

void IDCardPanel::EjectCard() {
    LOG_INFO("Ejecting ID card");
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    if (m_idCardReader->EjectCard()) {
        LOG_INFO("ID card ejection requested");
        // 清空当前卡片信息
        m_hasCardInfo = false;
        m_currentCardInfo.Clear();
    } else {
        LOG_ERROR("Failed to eject ID card");
    }
}

void IDCardPanel::LoadIDCardLibrary() {
    LOG_INFO("Loading ID card reader library: " + m_libraryPath);

    if (m_idCardReader && IsDeviceConnected()) {
        DisconnectDevice();
    }

    auto customReader = DeviceFactory::CreateIDCardReader("custom");
    if (!customReader) {
        LOG_ERROR("Failed to create custom ID card reader instance");
        return;
    }

    if (!customReader->Initialize()) {
        LOG_ERROR("Failed to initialize custom ID card reader instance");
        return;
    }

    if (!LoadPanelIDCardLibrary(customReader.get(), m_libraryPath)) {
        customReader->Shutdown();
        return;
    }

    if (!OpenPanelIDCardDevice(customReader.get(), m_devicePort)) {
        LOG_ERROR("Failed to open ID card device after library reload");
        customReader->Shutdown();
        return;
    }

    m_customReader = std::move(customReader);
    m_idCardReader = m_customReader.get();
    m_hasCardInfo = false;
    m_currentCardInfo.Clear();
    LOG_INFO("ID card reader library reloaded successfully: " + m_libraryPath);
}

bool IDCardPanel::IsDeviceConnected() const {
    return m_idCardReader && m_idCardReader->IsConnected();
}

std::string IDCardPanel::GetDeviceStatusText() const {
    if (!m_idCardReader) {
        return "设备未设置";
    }
    
    if (IsDeviceConnected()) {
        return "已连接";
    } else {
        return "未连接";
    }
}

ImVec4 IDCardPanel::GetDeviceStatusColor() const {
    if (!m_idCardReader) {
        return ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // 灰色
    }
    
    if (IsDeviceConnected()) {
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 绿色
    } else {
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // 红色
    }
}

} // namespace AsTestTool
