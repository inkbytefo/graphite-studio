#include "gui/LayersPanel.h"
#include "imgui_internal.h"
#include "core/Layer.h"
#include "core/LayerStack.h"
#include "core/DocumentCommands.h"
#include <iostream>

namespace gui {

void LayersPanel::Render(CanvasView& canvasView) {
    ImGui::Begin("Layers");

    if (!canvasView.IsImageLoaded()) {
        ImGui::Text("No document open");
        ImGui::End();
        return;
    }

    core::LayerStack& stack = canvasView.GetLayerStack();
    int layerCount = stack.GetCount();
    int selectedIndex = stack.GetSelectedIndex();
    core::Layer* selectedLayer = stack.GetSelectedLayer();

    // ── 1. Top controls: Blend mode and Opacity ──
    ImGui::PushItemWidth(120.0f);
    
    // Convert current selected layer blend mode
    int currentMode = selectedLayer ? static_cast<int>(selectedLayer->blendMode) : 0;
    
    if (ImGui::Combo("##BlendMode", &currentMode, core::GetBlendModeNames(), core::kBlendModeCount)) {
        if (selectedLayer && !selectedLayer->locked) {
            canvasView.GetHistoryManager().RecordState(stack, "Layer Blend Mode");
            selectedLayer->blendMode = static_cast<core::BlendMode>(currentMode);
        }
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::PushItemWidth(100.0f);
    float opacityPct = selectedLayer ? (selectedLayer->opacity * 100.0f) : 100.0f;
    ImGui::Text("Opacity:");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##LayerOpacity", &opacityPct, 0.0f, 100.0f, "%.0f%%")) {
        if (selectedLayer && !selectedLayer->locked) {
            selectedLayer->opacity = opacityPct / 100.0f;
        }
    }
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        if (selectedLayer && !selectedLayer->locked) {
            canvasView.GetHistoryManager().RecordState(stack, "Layer Opacity");
        }
    }
    ImGui::PopItemWidth();

    ImGui::Separator();

    // ── 2. Layer list ──
    ImVec2 listSize = ImVec2(-1, ImGui::GetContentRegionAvail().y - 36.0f);
    ImGui::BeginChild("LayerList", listSize, true);

    // Photoshop lists layers in reverse stack order: top-most layer at the top of list
    int draggedIndex = -1;
    int targetDropIndex = -1;

    for (int i = layerCount - 1; i >= 0; --i) {
        core::Layer* layer = stack.GetLayer(i);
        ImGui::PushID(layer->texture.GetId());

        bool isSelected = (selectedIndex == i);

        // Styling for selected row
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.176f, 0.365f, 0.667f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.176f, 0.365f, 0.667f, 0.7f));
        }

        // A. Visibility eye checkbox (custom drawn or simple check)
        bool visible = layer->visible;
        if (ImGui::Checkbox("##vis", &visible)) {
            canvasView.GetHistoryManager().RecordState(stack, visible ? "Show Layer" : "Hide Layer");
            layer->visible = visible;
        }
        ImGui::SameLine();

        // B. Layer Selectable Row
        char label[128];
        snprintf(label, sizeof(label), "%s%s", layer->name.c_str(), layer->locked ? "  [Locked]" : "");

        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 22.0f))) {
            stack.SetSelectedIndex(i);
        }

        // C. Double-click to rename layer
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            ImGui::OpenPopup("RenameLayerPopup");
        }

        if (ImGui::BeginPopup("RenameLayerPopup")) {
            static char nameBuf[64];
            if (ImGui::IsWindowAppearing()) {
                snprintf(nameBuf, sizeof(nameBuf), "%s", layer->name.c_str());
            }
            ImGui::Text("Rename Layer:");
            if (ImGui::InputText("##newName", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (strlen(nameBuf) > 0) {
                    canvasView.GetHistoryManager().RecordState(stack, "Rename Layer");
                    layer->name = nameBuf;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // D. Drag & Drop Reordering (Photoshop-accurate)
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
            ImGui::SetDragDropPayload("LAYER_INDEX", &i, sizeof(int));
            ImGui::Text("Moving layer: %s", layer->name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_INDEX")) {
                int sourceIdx = *(const int*)payload->Data;
                draggedIndex = sourceIdx;
                targetDropIndex = i;
            }
            ImGui::EndDragDropTarget();
        }

        if (isSelected) {
            ImGui::PopStyleColor(2);
        }

        ImGui::PopID();
    }

    // Apply drag & drop reordering if happened
    if (draggedIndex != -1 && targetDropIndex != -1) {
        canvasView.GetHistoryManager().RecordState(stack, "Reorder Layers");
        stack.MoveLayer(draggedIndex, targetDropIndex);
    }

    ImGui::EndChild();

    // ── 3. Bottom action buttons ──
    float btnSize = 24.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));

    // A. Add Layer button
    if (ImGui::Button("+", ImVec2(btnSize, btnSize))) {
        core::commands::NewLayer(stack, canvasView.GetHistoryManager(), canvasView.GetImageWidth(), canvasView.GetImageHeight());
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Create New Layer");

    ImGui::SameLine();

    // B. Delete Layer button
    if (ImGui::Button("-", ImVec2(btnSize, btnSize))) {
        core::commands::DeleteLayer(stack, canvasView.GetHistoryManager(), selectedIndex);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Selected Layer");

    ImGui::SameLine();

    // C. Duplicate Layer button
    if (ImGui::Button("D", ImVec2(btnSize, btnSize))) {
        core::commands::DuplicateLayer(stack, canvasView.GetHistoryManager(), selectedIndex);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate Selected Layer");

    ImGui::PopStyleVar();

    ImGui::End();
}

} // namespace gui
