#include "gui/PropertiesPanel.h"
#include "core/Layer.h"
#include "core/LayerStack.h"
#include <iostream>

namespace gui {

void PropertiesPanel::Render(CanvasView& canvasView) {
    ImGui::Begin("Properties");

    if (!canvasView.IsImageLoaded()) {
        ImGui::Text("No document open");
        ImGui::End();
        return;
    }

    core::LayerStack& stack = canvasView.GetLayerStack();
    core::Layer* selectedLayer = stack.GetSelectedLayer();

    // ── Document Properties Section ──
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.600f, 0.600f, 0.600f, 1.0f));
    ImGui::Text("PROPERTIES");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    if (!selectedLayer) {
        ImGui::Text("No layer selected");
        ImGui::End();
        return;
    }

    // Transform section header
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        float w = static_cast<float>(selectedLayer->width);
        float h = static_cast<float>(selectedLayer->height);
        float x = 0.0f; // Layer offset (placeholder for future move offset tool)
        float y = 0.0f;

        ImGui::PushItemWidth(80.0f);

        ImGui::Text("X:");  ImGui::SameLine(); ImGui::InputFloat("##PosX", &x, 0, 0, "%.1f", ImGuiInputTextFlags_ReadOnly);
        ImGui::Text("Y:");  ImGui::SameLine(); ImGui::InputFloat("##PosY", &y, 0, 0, "%.1f", ImGuiInputTextFlags_ReadOnly);
        ImGui::Text("W:");  ImGui::SameLine(); ImGui::InputFloat("##Width", &w, 0, 0, "%.1f", ImGuiInputTextFlags_ReadOnly);
        ImGui::Text("H:");  ImGui::SameLine(); ImGui::InputFloat("##Height", &h, 0, 0, "%.1f", ImGuiInputTextFlags_ReadOnly);

        ImGui::PopItemWidth();
        ImGui::Unindent(8.0f);
    }

    ImGui::Dummy(ImVec2(0, 4.0f));

    // Appearance section
    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        // Opacity
        float opacityPct = selectedLayer->opacity * 100.0f;
        ImGui::PushItemWidth(-1);
        ImGui::Text("Opacity:");
        if (ImGui::SliderFloat("##Opacity", &opacityPct, 0.0f, 100.0f, "%.0f%%")) {
            if (!selectedLayer->locked) {
                selectedLayer->opacity = opacityPct / 100.0f;
            }
        }
        ImGui::PopItemWidth();

        // Blend Mode
        ImGui::Dummy(ImVec2(0, 4.0f));
        ImGui::Text("Blend Mode:");
        ImGui::PushItemWidth(-1);
        int currentMode = static_cast<int>(selectedLayer->blendMode);
        if (ImGui::Combo("##BlendModeCombo", &currentMode, core::GetBlendModeNames(), core::kBlendModeCount)) {
            if (!selectedLayer->locked) {
                canvasView.GetHistoryManager().RecordState(stack, "Layer Blend Mode");
                selectedLayer->blendMode = static_cast<core::BlendMode>(currentMode);
            }
        }
        ImGui::PopItemWidth();

        ImGui::Unindent(8.0f);
    }

    ImGui::End();
}

} // namespace gui
