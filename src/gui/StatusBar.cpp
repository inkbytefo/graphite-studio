#include "gui/StatusBar.h"
#include <cstdio>

namespace gui {

void StatusBar::Render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float barHeight = 24.0f;
    ImVec2 barPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - barHeight);
    ImVec2 barSize = ImVec2(viewport->WorkSize.x, barHeight);

    ImGui::SetNextWindowPos(barPos);
    ImGui::SetNextWindowSize(barSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.118f, 0.118f, 0.118f, 1.0f)); // #1e1e1e
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.600f, 0.600f, 0.600f, 1.0f));     // #999999

    ImGui::Begin("##StatusBar", nullptr, flags);

    if (m_HasDocument) {
        // Zoom percentage
        char zoomText[32];
        snprintf(zoomText, sizeof(zoomText), "%.0f%%", m_Zoom * 100.0f);
        ImGui::Text("%s", zoomText);

        ImGui::SameLine(0, 16.0f);
        ImGui::TextUnformatted("|");
        ImGui::SameLine(0, 16.0f);

        // Document dimensions
        char dimText[64];
        snprintf(dimText, sizeof(dimText), "%d x %d", m_DocWidth, m_DocHeight);
        ImGui::Text("%s", dimText);

        ImGui::SameLine(0, 16.0f);
        ImGui::TextUnformatted("|");
        ImGui::SameLine(0, 16.0f);

        // Color mode
        ImGui::Text("RGB/8");

        ImGui::SameLine(0, 16.0f);
        ImGui::TextUnformatted("|");
        ImGui::SameLine(0, 16.0f);

        // Mouse position
        char posText[64];
        snprintf(posText, sizeof(posText), "X: %.0f  Y: %.0f", m_MouseX, m_MouseY);
        ImGui::Text("%s", posText);
    } else {
        ImGui::Text("Ready");
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

} // namespace gui
