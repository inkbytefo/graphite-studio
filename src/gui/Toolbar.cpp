#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "gui/Toolbar.h"
#include "imgui_internal.h"
#include "gui/IconHelper.h"

namespace gui {

// Define and initialize static members
ImVec4 Toolbar::ForegroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Default Black
ImVec4 Toolbar::BackgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default White

float Toolbar::BrushSize = 20.0f;
float Toolbar::BrushHardness = 0.5f;
float Toolbar::BrushOpacity = 1.0f;

struct ToolDef {
    const char* iconFile;  // SVG filename under resources/icons/
    const char* name;      // Full name for tooltip
    const char* keyLabel;  // Keyboard shortcut label
    ImGuiKey    hotkey;    // Keyboard shortcut key
};

static const ToolDef s_Tools[] = {
    { "move.svg", "Move Tool",       "V", ImGuiKey_V },
    { "marquee.svg", "Marquee Select",  "M", ImGuiKey_M },
    { "lasso.svg", "Lasso Tool",      "L", ImGuiKey_L },
    { "magic_wand.svg", "Magic Wand",      "W", ImGuiKey_W },
    { "crop.svg", "Crop Tool",       "C", ImGuiKey_C },
    { "eyedropper.svg", "Eyedropper",      "I", ImGuiKey_I },
    { "healing_brush.svg", "Healing Brush",   "J", ImGuiKey_J },
    { "brush.svg", "Brush Tool",      "B", ImGuiKey_B },
    { "clone_stamp.svg", "Clone Stamp",     "S", ImGuiKey_S },
    { "history_brush.svg", "History Brush",   "Y", ImGuiKey_Y },
    { "eraser.svg", "Eraser Tool",      "E", ImGuiKey_E },
    { "gradient.svg", "Gradient Tool",   "G", ImGuiKey_G },
    { "dodge.svg", "Dodge Tool",      "O", ImGuiKey_O },
    { "pen.svg", "Pen Tool",        "P", ImGuiKey_P },
    { "text.svg", "Text Tool",       "T", ImGuiKey_T },
    { "path_selection.svg", "Path Selection",  "A", ImGuiKey_A },
    { "shape.svg", "Shape Tool",      "U", ImGuiKey_U },
    { "hand.svg", "Hand Tool",       "H", ImGuiKey_H },
    { "zoom.svg", "Zoom Tool",       "Z", ImGuiKey_Z },
};

static const int s_ToolCount = sizeof(s_Tools) / sizeof(s_Tools[0]);

const char* Toolbar::GetSelectedToolName() const {
    if (m_SelectedTool >= 0 && m_SelectedTool < s_ToolCount) {
        return s_Tools[m_SelectedTool].name;
    }
    return "Unknown";
}

void Toolbar::Render() {
    ImGuiIO& io = ImGui::GetIO();

    // Handle keyboard shortcuts for tool selection (only when no modifier keys are pressed)
    if (!io.KeyCtrl && !io.KeyAlt && !io.KeyShift) {
        for (int i = 0; i < s_ToolCount; ++i) {
            if (ImGui::IsKeyPressed(s_Tools[i].hotkey)) {
                m_SelectedTool = i;
            }
        }

        // D: Reset Colors to Black & White
        if (ImGui::IsKeyPressed(ImGuiKey_D)) {
            ForegroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            BackgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // X: Swap Colors
        if (ImGui::IsKeyPressed(ImGuiKey_X)) {
            std::swap(ForegroundColor, BackgroundColor);
        }
    }

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
        if (ImGui::Button("##tool_btn", ImVec2(buttonSize, buttonSize))) {
            m_SelectedTool = i;
        }

        // Draw the SVG icon on top of the button
        auto iconTex = IconHelper::GetIcon(s_Tools[i].iconFile, 22, 22);
        if (iconTex) {
            ImVec2 btnMin = ImGui::GetItemRectMin();
            ImVec2 iconMin = ImVec2(btnMin.x + 3.0f, btnMin.y + 3.0f);
            ImVec2 iconMax = ImVec2(iconMin.x + 22.0f, iconMin.y + 22.0f);
            
            ImGui::GetWindowDrawList()->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(iconTex->GetId())),
                iconMin, iconMax, ImVec2(0,0), ImVec2(1,1), IM_COL32(255, 255, 255, 255)
            );
        }

        // Tooltip showing tool name and hotkey (V, M, etc.)
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("%s (%s)", s_Tools[i].name, s_Tools[i].keyLabel);
        }

        ImGui::PopID();
        ImGui::PopStyleColor(3);
    }

    // Foreground/Background color indicator at bottom
    ImGui::Dummy(ImVec2(0, 8.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    // Draw the classic foreground/background color squares with swap/reset buttons
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float sqSize = 14.0f;

    ImVec2 bgMin = ImVec2(cursor.x + 10.0f, cursor.y + 10.0f);
    ImVec2 bgMax = ImVec2(bgMin.x + sqSize, bgMin.y + sqSize);

    ImVec2 fgMin = ImVec2(cursor.x + 2.0f, cursor.y + 2.0f);
    ImVec2 fgMax = ImVec2(fgMin.x + sqSize, fgMin.y + sqSize);

    // Make background square clickable
    ImGui::SetCursorScreenPos(bgMin);
    if (ImGui::InvisibleButton("##bgButton", ImVec2(sqSize, sqSize))) {
        ImGui::OpenPopup("Background Color Picker");
    }
    ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(BackgroundColor);
    drawList->AddRectFilled(bgMin, bgMax, bgCol);
    drawList->AddRect(bgMin, bgMax, IM_COL32(128, 128, 128, 255));

    // Make foreground square clickable
    ImGui::SetCursorScreenPos(fgMin);
    if (ImGui::InvisibleButton("##fgButton", ImVec2(sqSize, sqSize))) {
        ImGui::OpenPopup("Foreground Color Picker");
    }
    ImU32 fgCol = ImGui::ColorConvertFloat4ToU32(ForegroundColor);
    drawList->AddRectFilled(fgMin, fgMax, fgCol);
    drawList->AddRect(fgMin, fgMax, IM_COL32(128, 128, 128, 255));

    // Draw the swap button (⇆)
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + 22.0f, cursor.y + 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Button("##swapBtn", ImVec2(12.0f, 12.0f))) {
        std::swap(ForegroundColor, BackgroundColor);
    }
    auto swapIcon = IconHelper::GetIcon("swap_color.svg", 12, 12);
    if (swapIcon) {
        ImVec2 btnMin = ImGui::GetItemRectMin();
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<void*>(static_cast<intptr_t>(swapIcon->GetId())),
            ImVec2(btnMin.x, btnMin.y),
            ImVec2(btnMin.x + 12.0f, btnMin.y + 12.0f)
        );
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Swap Colors (X)");
    }

    // Draw the reset button (⛶)
    ImGui::SetCursorScreenPos(ImVec2(cursor.x + 2.0f, cursor.y + 22.0f));
    if (ImGui::Button("##resetBtn", ImVec2(12.0f, 12.0f))) {
        ForegroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        BackgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    auto resetIcon = IconHelper::GetIcon("reset_color.svg", 12, 12);
    if (resetIcon) {
        ImVec2 btnMin = ImGui::GetItemRectMin();
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<void*>(static_cast<intptr_t>(resetIcon->GetId())),
            ImVec2(btnMin.x, btnMin.y),
            ImVec2(btnMin.x + 12.0f, btnMin.y + 12.0f)
        );
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reset Colors (D)");
    }
    ImGui::PopStyleVar();

    // Foreground Color Picker Popup
    if (ImGui::BeginPopup("Foreground Color Picker")) {
        ImGui::Text("Foreground Color");
        ImGui::ColorPicker4("##fgPicker", (float*)&ForegroundColor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Background Color Picker Popup
    if (ImGui::BeginPopup("Background Color Picker")) {
        ImGui::Text("Background Color");
        ImGui::ColorPicker4("##bgPicker", (float*)&BackgroundColor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Reserve space so ImGui knows about our custom drawing
    ImGui::SetCursorScreenPos(ImVec2(cursor.x, cursor.y));
    ImGui::Dummy(ImVec2(buttonSize, sqSize + 22.0f));

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace gui
