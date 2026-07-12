#include "gui/ColorPanel.h"
#include "gui/Toolbar.h"
#include "imgui.h"

namespace gui {

void ColorPanel::Render(CanvasView& canvasView) {
    ImGui::Begin("Color");

    static int activeColorTarget = 0; // 0 = Foreground, 1 = Background

    ImGui::Text("Edit Target:");
    ImGui::SameLine();
    ImGui::RadioButton("Foreground", &activeColorTarget, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Background", &activeColorTarget, 1);

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    ImVec4& targetColor = (activeColorTarget == 0) ? Toolbar::ForegroundColor : Toolbar::BackgroundColor;

    // Color picker wheel (HSV style)
    ImGui::PushItemWidth(-1);
    ImGui::ColorPicker3("##WheelPicker", (float*)&targetColor, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
    ImGui::PopItemWidth();

    ImGui::Dummy(ImVec2(0, 8.0f));
    ImGui::Text("Swatches");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    // Preset color swatches
    static const struct Swatch {
        const char* name;
        ImVec4 color;
    } s_Swatches[] = {
        { "Black",      ImVec4(0.0f, 0.0f, 0.0f, 1.0f) },
        { "White",      ImVec4(1.0f, 1.0f, 1.0f, 1.0f) },
        { "Dark Gray",  ImVec4(0.2f, 0.2f, 0.2f, 1.0f) },
        { "Light Gray", ImVec4(0.7f, 0.7f, 0.7f, 1.0f) },
        { "Red",        ImVec4(0.9f, 0.1f, 0.1f, 1.0f) },
        { "Orange",     ImVec4(1.0f, 0.5f, 0.0f, 1.0f) },
        { "Yellow",     ImVec4(1.0f, 0.9f, 0.0f, 1.0f) },
        { "Green",      ImVec4(0.1f, 0.8f, 0.1f, 1.0f) },
        { "Cyan",       ImVec4(0.0f, 0.8f, 0.8f, 1.0f) },
        { "Blue",       ImVec4(0.1f, 0.4f, 0.9f, 1.0f) },
        { "Purple",     ImVec4(0.5f, 0.1f, 0.8f, 1.0f) },
        { "Magenta",    ImVec4(0.9f, 0.1f, 0.6f, 1.0f) },
        { "Brown",      ImVec4(0.5f, 0.3f, 0.1f, 1.0f) },
        { "Pink",       ImVec4(1.0f, 0.6f, 0.7f, 1.0f) },
        { "Navy",       ImVec4(0.0f, 0.0f, 0.5f, 1.0f) },
        { "Gold",       ImVec4(0.85f, 0.7f, 0.0f, 1.0f) }
    };
    static const int swatchCount = sizeof(s_Swatches) / sizeof(s_Swatches[0]);

    float buttonSize = 22.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = static_cast<int>(panelWidth / (buttonSize + 4.0f));
    if (columns < 1) columns = 1;

    for (int i = 0; i < swatchCount; ++i) {
        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button, s_Swatches[i].color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(s_Swatches[i].color.x * 1.1f, s_Swatches[i].color.y * 1.1f, s_Swatches[i].color.z * 1.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, s_Swatches[i].color);
        
        if (ImGui::Button("##swatch", ImVec2(buttonSize, buttonSize))) {
            targetColor = s_Swatches[i].color;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", s_Swatches[i].name);
        }

        ImGui::PopStyleColor(3);
        ImGui::PopID();

        if ((i + 1) % columns != 0 && i < swatchCount - 1) {
            ImGui::SameLine(0.0f, 4.0f);
        }
    }

    ImGui::End();
}

} // namespace gui
