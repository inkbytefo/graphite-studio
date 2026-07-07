#include "gui/HistoryPanel.h"
#include <iostream>

namespace gui {

void HistoryPanel::Render(CanvasView& canvasView) {
    ImGui::Begin("History");

    if (!canvasView.IsImageLoaded()) {
        ImGui::Text("No document open");
        ImGui::End();
        return;
    }

    core::HistoryManager& history = canvasView.GetHistoryManager();
    const auto& actions = history.GetActionHistory();
    int currentIdx = history.GetCurrentStateIndex();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.600f, 0.600f, 0.600f, 1.0f));
    ImGui::Text("HISTORY STATES");
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4.0f));

    ImVec2 listSize = ImVec2(-1, -1);
    ImGui::BeginChild("HistoryStatesList", listSize, true);

    for (int i = 0; i < static_cast<int>(actions.size()); ++i) {
        bool isActive = (currentIdx == i);
        bool isForwardState = (i > currentIdx);

        ImGui::PushID(i);

        // Styling for future states (grayed out) or active state
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.176f, 0.365f, 0.667f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.176f, 0.365f, 0.667f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        } else if (isForwardState) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.450f, 0.450f, 0.450f, 1.0f)); // Grayed out
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.850f, 0.850f, 0.850f, 1.0f)); // Normal past states
        }

        // Draw Selectable row
        char label[128];
        snprintf(label, sizeof(label), "  %s", actions[i].c_str());

        if (ImGui::Selectable(label, isActive, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 20.0f))) {
            history.JumpToState(i, canvasView.GetLayerStack());
        }

        // Pop style colors
        if (isActive) {
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PopStyleColor();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();
}

} // namespace gui
