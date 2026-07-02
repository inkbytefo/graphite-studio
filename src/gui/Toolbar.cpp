#include "gui/Toolbar.h"
#include "imgui_internal.h"

namespace gui {

struct ToolDef {
    const char* shortcut;  // Single letter shown on button
    const char* name;      // Full name for tooltip
};

static const ToolDef s_Tools[] = {
    { "V", "Move Tool" },
    { "M", "Marquee Select" },
    { "L", "Lasso Tool" },
    { "W", "Magic Wand" },
    { "C", "Crop Tool" },
    { "I", "Eyedropper" },
    { "J", "Healing Brush" },
    { "B", "Brush Tool" },
    { "S", "Clone Stamp" },
    { "Y", "History Brush" },
    { "E", "Eraser Tool" },
    { "G", "Gradient Tool" },
    { "O", "Dodge Tool" },
    { "P", "Pen Tool" },
    { "T", "Text Tool" },
    { "A", "Path Selection" },
    { "U", "Shape Tool" },
    { "H", "Hand Tool" },
    { "Z", "Zoom Tool" },
};

static const int s_ToolCount = sizeof(s_Tools) / sizeof(s_Tools[0]);

const char* Toolbar::GetSelectedToolName() const {
    if (m_SelectedTool >= 0 && m_SelectedTool < s_ToolCount) {
        return s_Tools[m_SelectedTool].name;
    }
    return "Unknown";
}

void Toolbar::Render() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.176f, 0.176f, 0.176f, 1.0f)); // #2d2d2d

    ImGui::Begin("##Toolbar", nullptr, flags);

    float buttonSize = 28.0f;
    ImVec4 selectedColor    = ImVec4(0.176f, 0.365f, 0.667f, 1.0f); // #2d5daa
    ImVec4 normalColor      = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);      // transparent
    ImVec4 hoverColor       = ImVec4(0.243f, 0.243f, 0.243f, 1.0f); // #3e3e3e

    for (int i = 0; i < s_ToolCount; ++i) {
        bool isSelected = (m_SelectedTool == i);

        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, selectedColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selectedColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, selectedColor);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, normalColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, selectedColor);
        }

        ImGui::PushID(i);
        if (ImGui::Button(s_Tools[i].shortcut, ImVec2(buttonSize, buttonSize))) {
            m_SelectedTool = i;
        }

        // Tooltip
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s (%s)", s_Tools[i].name, s_Tools[i].shortcut);
        }

        ImGui::PopID();
        ImGui::PopStyleColor(3);
    }

    // Foreground/Background color indicator at bottom
    ImGui::Dummy(ImVec2(0, 8.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    // Draw the classic foreground/background color squares
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float sqSize = 18.0f;

    // Background color (white) - drawn slightly offset behind
    ImVec2 bgMin = ImVec2(cursor.x + 10.0f, cursor.y + 10.0f);
    ImVec2 bgMax = ImVec2(bgMin.x + sqSize, bgMin.y + sqSize);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(255, 255, 255, 255));
    drawList->AddRect(bgMin, bgMax, IM_COL32(128, 128, 128, 255));

    // Foreground color (black) - drawn on top
    ImVec2 fgMin = ImVec2(cursor.x + 2.0f, cursor.y + 2.0f);
    ImVec2 fgMax = ImVec2(fgMin.x + sqSize, fgMin.y + sqSize);
    drawList->AddRectFilled(fgMin, fgMax, IM_COL32(0, 0, 0, 255));
    drawList->AddRect(fgMin, fgMax, IM_COL32(128, 128, 128, 255));

    // Reserve space so ImGui knows about our custom drawing
    ImGui::Dummy(ImVec2(buttonSize, sqSize + 14.0f));

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace gui
