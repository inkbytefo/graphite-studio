#include "gui/Workspace.h"
#include "imgui_internal.h"

namespace gui {

void SetupDefaultLayout(ImGuiID dockspace_id, bool force) {
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
    
    // Only configure if explicitly forced OR if the dock node is empty (no splits, no windows)
    if (!force) {
        if (node != nullptr && (node->ChildNodes[0] != nullptr || node->Windows.Size > 0)) {
            return; // Layout already configured or loaded from imgui.ini
        }
    }

    // Clear any existing layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    // ┌────────┬──────────────────────────┬─────────────┐
    // │Toolbar │       Canvas             │  Right side │
    // │  (5%)  │        (70%)             │   (25%)     │
    // │        │                          ├─────────────┤
    // │        │                          │             │
    // │        │                          │   Layers    │
    // └────────┴──────────────────────────┴─────────────┘

    ImGuiID dock_main = dockspace_id;

    // Split left toolbar (narrow, ~48px equivalent = ~4% of window)
    ImGuiID dock_toolbar;
    ImGuiID dock_remaining;
    ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.04f, &dock_toolbar, &dock_remaining);

    // Split top options bar from the remaining content area (~36px height)
    ImGuiID dock_options;
    ImGuiID dock_content_area;
    ImGui::DockBuilderSplitNode(dock_remaining, ImGuiDir_Up, 0.045f, &dock_options, &dock_content_area);

    // Split right panel from the content area
    ImGuiID dock_right;
    ImGuiID dock_center;
    ImGui::DockBuilderSplitNode(dock_content_area, ImGuiDir_Right, 0.22f, &dock_right, &dock_center);

    // Split right panel into top (Properties & History) and bottom (Layers)
    ImGuiID dock_right_top;
    ImGuiID dock_right_bottom;
    ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.55f, &dock_right_bottom, &dock_right_top);

    // Dock windows into their respective nodes
    ImGui::DockBuilderDockWindow("##Toolbar", dock_toolbar);
    ImGui::DockBuilderDockWindow("##OptionsBar", dock_options);
    ImGui::DockBuilderDockWindow("Canvas", dock_center);
    ImGui::DockBuilderDockWindow("Properties", dock_right_top);
    ImGui::DockBuilderDockWindow("History", dock_right_top);
    ImGui::DockBuilderDockWindow("Color", dock_right_top);
    ImGui::DockBuilderDockWindow("Layers", dock_right_bottom);

    // Make toolbar and options nodes non-resizable and hide their tab bars
    ImGuiDockNode* toolbarNode = ImGui::DockBuilderGetNode(dock_toolbar);
    if (toolbarNode) {
        toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
        toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoResize;
    }

    ImGuiDockNode* optionsNode = ImGui::DockBuilderGetNode(dock_options);
    if (optionsNode) {
        optionsNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
        optionsNode->LocalFlags |= ImGuiDockNodeFlags_NoResize;
    }

    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace gui
