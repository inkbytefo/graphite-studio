#include "gui/PropertiesPanel.h"

namespace gui {

void PropertiesPanel::Render() {
    ImGui::Begin("Properties");

    // ── Document Properties Section ──
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.600f, 0.600f, 0.600f, 1.0f));
    ImGui::Text("PROPERTIES");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    // Transform section header
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        float w = 0.0f, h = 0.0f, x = 0.0f, y = 0.0f;
        float angle = 0.0f;

        ImGui::PushItemWidth(80.0f);

        ImGui::Text("X:");  ImGui::SameLine(); ImGui::InputFloat("##PosX", &x, 0, 0, "%.1f");
        ImGui::Text("Y:");  ImGui::SameLine(); ImGui::InputFloat("##PosY", &y, 0, 0, "%.1f");
        ImGui::Text("W:");  ImGui::SameLine(); ImGui::InputFloat("##Width", &w, 0, 0, "%.1f");
        ImGui::Text("H:");  ImGui::SameLine(); ImGui::InputFloat("##Height", &h, 0, 0, "%.1f");

        ImGui::Dummy(ImVec2(0, 2.0f));
        ImGui::Text("Rotation:");
        ImGui::SameLine();
        ImGui::SliderFloat("##Angle", &angle, -180.0f, 180.0f, "%.1f");

        ImGui::PopItemWidth();
        ImGui::Unindent(8.0f);
    }

    ImGui::Dummy(ImVec2(0, 4.0f));

    // Appearance section
    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        static float opacity = 100.0f;
        ImGui::PushItemWidth(-1);
        ImGui::Text("Opacity:");
        ImGui::SliderFloat("##Opacity", &opacity, 0.0f, 100.0f, "%.0f%%");
        ImGui::PopItemWidth();

        ImGui::Unindent(8.0f);
    }

    ImGui::End();
}

} // namespace gui
