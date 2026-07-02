#include "gui/LayersPanel.h"
#include "imgui_internal.h"

namespace gui {

void LayersPanel::Render() {
    ImGui::Begin("Layers");

    // ── Top controls: Blend mode and Opacity (like Photoshop) ──
    ImGui::PushItemWidth(120.0f);
    static int blendMode = 0;
    const char* blendModes[] = { "Normal", "Dissolve", "Darken", "Multiply", "Color Burn",
                                  "Lighten", "Screen", "Color Dodge", "Overlay",
                                  "Soft Light", "Hard Light", "Difference", "Exclusion" };
    ImGui::Combo("##BlendMode", &blendMode, blendModes, IM_ARRAYSIZE(blendModes));
    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::PushItemWidth(80.0f);
    static float opacity = 100.0f;
    ImGui::Text("Opacity:");
    ImGui::SameLine();
    ImGui::SliderFloat("##LayerOpacity", &opacity, 0.0f, 100.0f, "%.0f%%");
    ImGui::PopItemWidth();

    ImGui::Separator();

    // ── Layer list ──
    ImVec2 listSize = ImVec2(-1, ImGui::GetContentRegionAvail().y - 36.0f);
    ImGui::BeginChild("LayerList", listSize, true);

    static int selectedLayer = 0;

    // Placeholder layers
    struct LayerInfo {
        const char* name;
        bool visible;
        bool locked;
    };

    static LayerInfo layers[] = {
        { "Background", true, true },
    };
    static int layerCount = 1;

    for (int i = layerCount - 1; i >= 0; --i) {
        ImGui::PushID(i);

        bool isSelected = (selectedLayer == i);

        // Layer row background
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.176f, 0.365f, 0.667f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.176f, 0.365f, 0.667f, 0.7f));
        }

        // Visibility toggle
        ImGui::Checkbox("##vis", &layers[i].visible);
        ImGui::SameLine();

        // Layer name (selectable)
        char label[128];
        snprintf(label, sizeof(label), "%s%s", layers[i].name, layers[i].locked ? "  [Locked]" : "");

        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 20.0f))) {
            selectedLayer = i;
        }

        if (isSelected) {
            ImGui::PopStyleColor(2);
        }

        ImGui::PopID();
    }

    ImGui::EndChild();

    // ── Bottom action buttons ──
    float btnSize = 24.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));

    if (ImGui::Button("+", ImVec2(btnSize, btnSize))) {
        // Add layer placeholder
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("New Layer");

    ImGui::SameLine();

    if (ImGui::Button("-", ImVec2(btnSize, btnSize))) {
        // Delete layer placeholder
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Layer");

    ImGui::SameLine();

    if (ImGui::Button("G", ImVec2(btnSize, btnSize))) {
        // Group placeholder
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Group Layers");

    ImGui::PopStyleVar();

    ImGui::End();
}

} // namespace gui
