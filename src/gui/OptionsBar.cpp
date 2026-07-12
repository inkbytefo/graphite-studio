#include "gui/OptionsBar.h"
#include "gui/Toolbar.h"

namespace gui {

void OptionsBar::Render(CanvasView& canvasView) {
    // In a Photoshop layout, the Options Bar occupies the full width at the top
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 6.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f)); // #2e2e2e
    
    ImGui::Begin("##OptionsBar", nullptr, flags);

    ActiveTool tool = canvasView.GetActiveTool();

    // Title / Icon representing the active tool
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    
    ImGui::Text("%s Tool |", GetToolName(tool));
    ImGui::PopStyleColor();
    ImGui::SameLine();

    // Show contextual tool settings
    if (tool == ActiveTool::Brush || tool == ActiveTool::Eraser) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Size:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("##BrushSize", &Toolbar::BrushSize, 1.0f, 200.0f, "%.0f px");
        
        ImGui::SameLine();
        ImGui::TextDisabled("  |  ");
        ImGui::SameLine();
        
        ImGui::Text("Hardness:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("##BrushHardness", &Toolbar::BrushHardness, 0.0f, 1.0f, "%.2f");
        
        ImGui::SameLine();
        ImGui::TextDisabled("  |  ");
        ImGui::SameLine();
        
        ImGui::Text("Opacity:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        float opacityPct = Toolbar::BrushOpacity * 100.0f;
        if (ImGui::SliderFloat("##BrushOpacity", &opacityPct, 1.0f, 100.0f, "%.0f%%")) {
            Toolbar::BrushOpacity = opacityPct / 100.0f;
        }
    } else if (tool == ActiveTool::Zoom) {
        if (ImGui::Button("Zoom In")) {
            canvasView.ZoomIn();
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) {
            canvasView.ZoomOut();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("  |  ");
        ImGui::SameLine();
        
        if (ImGui::Button("100%")) {
            canvasView.ZoomToActual();
        }
        ImGui::SameLine();
        if (ImGui::Button("Fit Screen")) {
            canvasView.ResetView();
        }
    } else if (tool == ActiveTool::Hand) {
        if (ImGui::Button("Reset View")) {
            canvasView.ResetView();
        }
    } else {
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("No properties to adjust for this tool.");
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

} // namespace gui
