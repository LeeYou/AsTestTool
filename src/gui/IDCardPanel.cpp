#include "gui/IDCardPanel.h"
#include "utils/Logger.h"
#include "devices/idcard/WindowsIDCardReader.h"
#include "imgui.h"
#include <cstring>

namespace AsTestTool {

IDCardPanel::IDCardPanel() {
    LOG_INFO("IDCardPanel created");
}

IDCardPanel::~IDCardPanel() {
    LOG_INFO("IDCardPanel destroyed");
}

void IDCardPanel::SetIDCardReader(std::shared_ptr<IIDCardReader> reader) {
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
    ImGui::Text("设备状态:");
    ImGui::SameLine();
    
    std::string statusText = GetDeviceStatusText();
    ImVec4 statusColor = GetDeviceStatusColor();
    ImGui::TextColored(statusColor, "%s", statusText.c_str());
    
    if (m_idCardReader) {
        ImGui::Text("设备信息: %s", m_idCardReader->GetDeviceInfo().c_str());
    }
    
    ImGui::Separator();
}

void IDCardPanel::RenderCardInfo() {
    ImGui::Text("身份证信息:");
    ImGui::Separator();
    
    // 添加调试信息
    ImGui::Text("调试: m_hasCardInfo=%s, IsValid=%s", 
                m_hasCardInfo ? "true" : "false",
                m_currentCardInfo.IsValid() ? "true" : "false");
    
    if (m_hasCardInfo && m_currentCardInfo.IsValid()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== 身份证信息已读取 ===");
        ImGui::Text("姓名: %s", m_currentCardInfo.name.c_str());
        ImGui::Text("性别: %s", m_currentCardInfo.gender.c_str());
        ImGui::Text("民族: %s", m_currentCardInfo.nation.c_str());
        ImGui::Text("出生日期: %s", m_currentCardInfo.birthDate.c_str());
        ImGui::Text("住址: %s", m_currentCardInfo.address.c_str());
        ImGui::Text("身份证号: %s", m_currentCardInfo.idNumber.c_str());
        ImGui::Text("签发机关: %s", m_currentCardInfo.issuingAuthority.c_str());
        ImGui::Text("有效期限: %s", m_currentCardInfo.validPeriod.c_str());
        
        ImGui::Separator();
        if (!m_currentCardInfo.photo.empty()) {
            ImGui::Text("照片: 已获取 (BASE64编码)");
            ImGui::Text("照片大小: %zu 字节", m_currentCardInfo.photo.size());
        } else {
            ImGui::Text("照片: 无");
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "=== 身份证信息未读取 ===");
        ImGui::Text("姓名: 未读取");
        ImGui::Text("性别: 未读取");
        ImGui::Text("民族: 未读取");
        ImGui::Text("出生日期: 未读取");
        ImGui::Text("住址: 未读取");
        ImGui::Text("身份证号: 未读取");
        ImGui::Text("签发机关: 未读取");
        ImGui::Text("有效期限: 未读取");
        
        ImGui::Separator();
        ImGui::Text("照片: 无");
    }
}

void IDCardPanel::RenderControls() {
    // 设备连接控制
    if (IsDeviceConnected()) {
        if (ImGui::Button("断开设备", ImVec2(100, 30))) {
            DisconnectDevice();
        }
    } else {
        if (ImGui::Button("连接设备", ImVec2(100, 30))) {
            ConnectDevice();
        }
    }
    
    ImGui::SameLine();
    
    // 读取身份证控制
    bool canRead = IsDeviceConnected() && m_idCardReader && m_idCardReader->HasCard();
    if (!canRead) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button("读取身份证", ImVec2(100, 30))) {
        ReadCard();
    }
    
    if (!canRead) {
        ImGui::EndDisabled();
    }
    
    ImGui::SameLine();
    
    // 弹出卡片控制
    bool canEject = IsDeviceConnected();
    if (!canEject) {
        ImGui::BeginDisabled();
    }
    
    if (ImGui::Button("弹出卡片", ImVec2(100, 30))) {
        EjectCard();
    }
    
    if (!canEject) {
        ImGui::EndDisabled();
    }
    
    ImGui::Separator();
    
    // 库设置按钮
    if (ImGui::Button("库设置", ImVec2(100, 30))) {
        m_showLibrarySettings = !m_showLibrarySettings;
    }
    
    ImGui::SameLine();
    
    // 重新加载库按钮
    if (ImGui::Button("重新加载库", ImVec2(100, 30))) {
        LoadIDCardLibrary();
    }
    
    ImGui::Separator();
}

void IDCardPanel::RenderLibrarySettings() {
    if (ImGui::Begin("身份证阅读器库设置", &m_showLibrarySettings)) {
        ImGui::Text("库文件路径:");
        ImGui::SameLine();
        
        char pathBuffer[512];
        strncpy_s(pathBuffer, m_libraryPath.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        
        if (ImGui::InputText("##LibraryPath", pathBuffer, sizeof(pathBuffer))) {
            m_libraryPath = std::string(pathBuffer);
        }
        
        ImGui::SameLine();
        if (ImGui::Button("浏览...", ImVec2(80, 20))) {
            // 打开文件对话框
            OPENFILENAMEA ofn;
            char szFile[512] = {0};
            
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "Dynamic Link Library (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrTitle = "选择身份证阅读器DLL文件";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            if (GetOpenFileNameA(&ofn)) {
                m_libraryPath = std::string(szFile);
                LOG_INFO("Selected DLL file: " + m_libraryPath);
            }
        }
        
        ImGui::Text("设备端口:");
        ImGui::SameLine();
        ImGui::InputInt("##DevicePort", &m_devicePort, 1, 10);
        
        ImGui::Separator();
        
        // 显示默认路径信息
        ImGui::Text("默认加载:");
        ImGui::BulletText("系统DLL: CMCC_IDCARD.DLL");
        ImGui::BulletText("Windows会自动在系统目录中查找");
        ImGui::BulletText("包括: System32, SysWOW64, PATH环境变量等");
        
        ImGui::Separator();
        
        if (ImGui::Button("应用设置", ImVec2(100, 30))) {
            LoadIDCardLibrary();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("关闭", ImVec2(100, 30))) {
            m_showLibrarySettings = false;
        }
        
        ImGui::Separator();
        ImGui::Text("说明:");
        ImGui::Text("- 库文件路径: 身份证阅读器DLL文件路径");
        ImGui::Text("- 设备端口: USB端口从1000开始，USB1填1001");
        ImGui::Text("- 支持的库: cmcc_idcard.dll 等标准接口库");
    }
    ImGui::End();
}

void IDCardPanel::ConnectDevice() {
    LOG_INFO("Connecting ID card device");
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    // 尝试转换为WindowsIDCardReader以访问OpenDevice方法
    auto windowsReader = std::dynamic_pointer_cast<WindowsIDCardReader>(m_idCardReader);
    if (windowsReader) {
        if (windowsReader->OpenDevice(m_devicePort)) {
            LOG_INFO("ID card device connected successfully");
        } else {
            LOG_ERROR("Failed to connect ID card device");
        }
    } else {
        LOG_ERROR("ID card reader is not a Windows implementation");
    }
}

void IDCardPanel::DisconnectDevice() {
    LOG_INFO("Disconnecting ID card device");
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    // 尝试转换为WindowsIDCardReader以访问CloseDevice方法
    auto windowsReader = std::dynamic_pointer_cast<WindowsIDCardReader>(m_idCardReader);
    if (windowsReader) {
        if (windowsReader->CloseDevice()) {
            LOG_INFO("ID card device disconnected successfully");
            // 清空当前卡片信息
            m_hasCardInfo = false;
            m_currentCardInfo.Clear();
        } else {
            LOG_ERROR("Failed to disconnect ID card device");
        }
    } else {
        LOG_ERROR("ID card reader is not a Windows implementation");
    }
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
    
    if (!m_idCardReader) {
        LOG_ERROR("No ID card reader device available");
        return;
    }
    
    // 尝试转换为WindowsIDCardReader以访问LoadLibrary方法
    auto windowsReader = std::dynamic_pointer_cast<WindowsIDCardReader>(m_idCardReader);
    if (windowsReader) {
        if (windowsReader->LoadLibrary(m_libraryPath)) {
            LOG_INFO("Library loaded successfully");
        } else {
            LOG_ERROR("Failed to load library: " + m_libraryPath);
        }
    } else {
        LOG_ERROR("ID card reader is not a Windows implementation");
    }
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
