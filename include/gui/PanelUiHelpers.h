#pragma once

#include <array>
#include <cstddef>
#include <initializer_list>
#include <imgui.h>

namespace AsTestTool::PanelUi {

struct InfoField {
    const char* label;
    const char* value;
};

namespace Constants {
inline constexpr float kButtonWidth = 100.0f;
inline constexpr float kButtonHeight = 30.0f;
inline constexpr float kPopupButtonWidth = 100.0f;
inline constexpr float kPopupButtonHeight = 30.0f;
inline constexpr float kPopupWidth = 520.0f;
inline constexpr float kPopupHeight = 320.0f;
inline constexpr float kBrowseButtonWidth = 80.0f;
}

namespace Layout {
inline void ContinueOnSameLineIfFits(float nextItemWidth) {
    const ImGuiStyle& style = ImGui::GetStyle();
    if (ImGui::GetContentRegionAvail().x >= nextItemWidth + style.ItemSpacing.x) {
        ImGui::SameLine();
    }
}

inline float GetInlineInputWidth(float trailingWidth = 0.0f, float minWidth = 160.0f) {
    const ImGuiStyle& style = ImGui::GetStyle();
    float width = ImGui::GetContentRegionAvail().x - trailingWidth;
    if (trailingWidth > 0.0f) {
        width -= style.ItemSpacing.x;
    }
    return width > minWidth ? width : minWidth;
}

inline void BeginInlineFieldRow(const char* label, float trailingWidth = 0.0f, float minWidth = 160.0f) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(GetInlineInputWidth(trailingWidth, minWidth));
}
}

namespace Sections {
inline void RenderSectionTitle(const char* title) {
    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::TextUnformatted(title);
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
}

inline void RenderStatusSummary(const char* label, const char* value, const ImVec4& color) {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", value);
}

inline void RenderInfoField(const InfoField& field) {
    ImGui::Text("%s %s", field.label, field.value);
}

inline void RenderDeviceStatusBlock(const char* statusText, const ImVec4& statusColor, const char* deviceInfo = nullptr) {
    RenderStatusSummary("设备状态:", statusText, statusColor);
    if (deviceInfo && deviceInfo[0] != '\0') {
        ImGui::Text("设备信息: %s", deviceInfo);
    }
}

inline void RenderStateBanner(const char* text, const ImVec4& color) {
    ImGui::TextColored(color, "%s", text);
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
}

inline void RenderDefaultLibraryHints(const char* systemDllName) {
    ImGui::BulletText("系统DLL: %s", systemDllName);
    ImGui::BulletText("系统会自动在系统目录中查找");
    ImGui::BulletText("包括: System32, SysWOW64, PATH环境变量等");
}

inline void RenderDescriptionLines(std::initializer_list<const char*> lines) {
    for (const char* line : lines) {
        ImGui::TextUnformatted(line);
    }
}

inline void RenderDefaultLibrarySection(const char* systemDllName) {
    RenderSectionTitle("默认加载:");
    RenderDefaultLibraryHints(systemDllName);
}

inline void RenderDescriptionSection(std::initializer_list<const char*> lines) {
    RenderSectionTitle("说明:");
    RenderDescriptionLines(lines);
}

template <size_t PairedCount, size_t FullWidthCount>
inline void RenderResponsiveInfoFields(
    const char* denseColumnsId,
    const std::array<InfoField, PairedCount>& pairedFields,
    const std::array<InfoField, FullWidthCount>& fullWidthFields,
    float denseThreshold = 420.0f
) {
    const bool useDenseLayout = ImGui::GetContentRegionAvail().x >= denseThreshold;
    if (useDenseLayout && PairedCount > 0) {
        ImGui::Columns(2, denseColumnsId, false);
        for (const auto& field : pairedFields) {
            Sections::RenderInfoField(field);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    } else {
        for (const auto& field : pairedFields) {
            Sections::RenderInfoField(field);
        }
    }

    for (const auto& field : fullWidthFields) {
        Sections::RenderInfoField(field);
    }
}

template <size_t PairedCount>
inline void RenderResponsiveInfoFields(
    const char* denseColumnsId,
    const std::array<InfoField, PairedCount>& pairedFields,
    float denseThreshold = 420.0f
) {
    RenderResponsiveInfoFields(denseColumnsId, pairedFields, std::array<InfoField, 0>{}, denseThreshold);
}
}

namespace Controls {
template <typename Callback>
inline void RenderActionButton(
    const char* label,
    Callback&& callback,
    bool disabled = false,
    float width = Constants::kButtonWidth,
    float height = Constants::kButtonHeight
) {
    if (disabled) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(label, ImVec2(width, height)) && !disabled) {
        callback();
    }

    if (disabled) {
        ImGui::EndDisabled();
    }
}

template <typename ConnectCallback, typename DisconnectCallback>
inline void RenderConnectionToggleButton(
    bool isConnected,
    ConnectCallback&& connectCallback,
    DisconnectCallback&& disconnectCallback,
    const char* connectLabel = "连接设备",
    const char* disconnectLabel = "断开设备",
    float width = Constants::kButtonWidth,
    float height = Constants::kButtonHeight
) {
    if (isConnected) {
        RenderActionButton(disconnectLabel, disconnectCallback, false, width, height);
    } else {
        RenderActionButton(connectLabel, connectCallback, false, width, height);
    }
}

inline bool RenderLabeledSliderInt(
    const char* label,
    const char* controlId,
    int* value,
    int minValue,
    int maxValue,
    float minWidth = 120.0f
) {
    Layout::BeginInlineFieldRow(label, 0.0f, minWidth);
    return ImGui::SliderInt(controlId, value, minValue, maxValue);
}

inline bool RenderLabeledSliderFloat(
    const char* label,
    const char* controlId,
    float* value,
    float minValue,
    float maxValue,
    float minWidth = 120.0f
) {
    Layout::BeginInlineFieldRow(label, 0.0f, minWidth);
    return ImGui::SliderFloat(controlId, value, minValue, maxValue);
}

inline bool RenderLabeledCombo(
    const char* label,
    const char* controlId,
    int* currentItem,
    const char* const items[],
    int itemCount,
    float minWidth = 140.0f
) {
    Layout::BeginInlineFieldRow(label, 0.0f, minWidth);
    return ImGui::Combo(controlId, currentItem, items, itemCount);
}

template <typename BrowseCallback>
inline bool RenderPathSelectorRow(
    const char* label,
    const char* inputId,
    char* pathBuffer,
    std::size_t bufferSize,
    BrowseCallback&& browseCallback,
    float minWidth = 220.0f
) {
    Layout::BeginInlineFieldRow(label, Constants::kBrowseButtonWidth, minWidth);
    bool changed = ImGui::InputText(inputId, pathBuffer, bufferSize);
    ImGui::SameLine();
    if (ImGui::Button("浏览...", ImVec2(Constants::kBrowseButtonWidth, 20.0f))) {
        browseCallback();
    }
    return changed;
}
}

namespace Popups {
inline void PrepareLibrarySettingsWindow() {
    ImGui::SetNextWindowSize(ImVec2(Constants::kPopupWidth, Constants::kPopupHeight), ImGuiCond_FirstUseEver);
}

template <typename ApplyCallback>
inline void RenderPopupActionButtons(bool* openState, ApplyCallback&& applyCallback) {
    if (ImGui::Button("应用设置", ImVec2(Constants::kPopupButtonWidth, Constants::kPopupButtonHeight))) {
        applyCallback();
    }

    Layout::ContinueOnSameLineIfFits(Constants::kPopupButtonWidth);

    if (ImGui::Button("关闭", ImVec2(Constants::kPopupButtonWidth, Constants::kPopupButtonHeight))) {
        if (openState) {
            *openState = false;
        }
    }
}

template <typename ApplyCallback>
inline void RenderSettingsActionSection(bool* openState, ApplyCallback&& applyCallback) {
    Sections::RenderSectionTitle("操作:");
    RenderPopupActionButtons(openState, applyCallback);
}
}

inline constexpr float kButtonWidth = Constants::kButtonWidth;
inline constexpr float kButtonHeight = Constants::kButtonHeight;
inline constexpr float kPopupButtonWidth = Constants::kPopupButtonWidth;
inline constexpr float kPopupButtonHeight = Constants::kPopupButtonHeight;
inline constexpr float kPopupWidth = Constants::kPopupWidth;
inline constexpr float kPopupHeight = Constants::kPopupHeight;
inline constexpr float kBrowseButtonWidth = Constants::kBrowseButtonWidth;

inline void ContinueOnSameLineIfFits(float nextItemWidth) {
    Layout::ContinueOnSameLineIfFits(nextItemWidth);
}

inline void RenderSectionTitle(const char* title) {
    Sections::RenderSectionTitle(title);
}

inline void RenderStatusSummary(const char* label, const char* value, const ImVec4& color) {
    Sections::RenderStatusSummary(label, value, color);
}

inline void RenderInfoField(const InfoField& field) {
    Sections::RenderInfoField(field);
}

inline void RenderDeviceStatusBlock(const char* statusText, const ImVec4& statusColor, const char* deviceInfo = nullptr) {
    Sections::RenderDeviceStatusBlock(statusText, statusColor, deviceInfo);
}

inline void RenderStateBanner(const char* text, const ImVec4& color) {
    Sections::RenderStateBanner(text, color);
}

inline void PrepareLibrarySettingsWindow() {
    Popups::PrepareLibrarySettingsWindow();
}

inline float GetInlineInputWidth(float trailingWidth = 0.0f, float minWidth = 160.0f) {
    return Layout::GetInlineInputWidth(trailingWidth, minWidth);
}

inline void BeginInlineFieldRow(const char* label, float trailingWidth = 0.0f, float minWidth = 160.0f) {
    Layout::BeginInlineFieldRow(label, trailingWidth, minWidth);
}

inline void RenderDefaultLibraryHints(const char* systemDllName) {
    Sections::RenderDefaultLibraryHints(systemDllName);
}

inline void RenderDescriptionLines(std::initializer_list<const char*> lines) {
    Sections::RenderDescriptionLines(lines);
}

template <typename Callback>
inline void RenderActionButton(
    const char* label,
    Callback&& callback,
    bool disabled = false,
    float width = kButtonWidth,
    float height = kButtonHeight
) {
    Controls::RenderActionButton(label, callback, disabled, width, height);
}

template <typename ConnectCallback, typename DisconnectCallback>
inline void RenderConnectionToggleButton(
    bool isConnected,
    ConnectCallback&& connectCallback,
    DisconnectCallback&& disconnectCallback,
    const char* connectLabel = "连接设备",
    const char* disconnectLabel = "断开设备",
    float width = kButtonWidth,
    float height = kButtonHeight
) {
    Controls::RenderConnectionToggleButton(isConnected, connectCallback, disconnectCallback, connectLabel, disconnectLabel, width, height);
}

inline bool RenderLabeledSliderInt(
    const char* label,
    const char* controlId,
    int* value,
    int minValue,
    int maxValue,
    float minWidth = 120.0f
) {
    return Controls::RenderLabeledSliderInt(label, controlId, value, minValue, maxValue, minWidth);
}

inline bool RenderLabeledSliderFloat(
    const char* label,
    const char* controlId,
    float* value,
    float minValue,
    float maxValue,
    float minWidth = 120.0f
) {
    return Controls::RenderLabeledSliderFloat(label, controlId, value, minValue, maxValue, minWidth);
}

inline bool RenderLabeledCombo(
    const char* label,
    const char* controlId,
    int* currentItem,
    const char* const items[],
    int itemCount,
    float minWidth = 140.0f
) {
    return Controls::RenderLabeledCombo(label, controlId, currentItem, items, itemCount, minWidth);
}

template <typename BrowseCallback>
inline bool RenderPathSelectorRow(
    const char* label,
    const char* inputId,
    char* pathBuffer,
    std::size_t bufferSize,
    BrowseCallback&& browseCallback,
    float minWidth = 220.0f
) {
    return Controls::RenderPathSelectorRow(label, inputId, pathBuffer, bufferSize, browseCallback, minWidth);
}

inline void RenderDefaultLibrarySection(const char* systemDllName) {
    Sections::RenderDefaultLibrarySection(systemDllName);
}

inline void RenderDescriptionSection(std::initializer_list<const char*> lines) {
    Sections::RenderDescriptionSection(lines);
}

template <size_t PairedCount, size_t FullWidthCount>
inline void RenderResponsiveInfoFields(
    const char* denseColumnsId,
    const std::array<InfoField, PairedCount>& pairedFields,
    const std::array<InfoField, FullWidthCount>& fullWidthFields,
    float denseThreshold = 420.0f
) {
    Sections::RenderResponsiveInfoFields(denseColumnsId, pairedFields, fullWidthFields, denseThreshold);
}

template <size_t PairedCount>
inline void RenderResponsiveInfoFields(
    const char* denseColumnsId,
    const std::array<InfoField, PairedCount>& pairedFields,
    float denseThreshold = 420.0f
) {
    Sections::RenderResponsiveInfoFields(denseColumnsId, pairedFields, denseThreshold);
}

template <typename ApplyCallback>
inline void RenderPopupActionButtons(bool* openState, ApplyCallback&& applyCallback) {
    Popups::RenderPopupActionButtons(openState, applyCallback);
}

template <typename ApplyCallback>
inline void RenderSettingsActionSection(bool* openState, ApplyCallback&& applyCallback) {
    Popups::RenderSettingsActionSection(openState, applyCallback);
}

}
