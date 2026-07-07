#pragma once

#include "imgui.h"
#include "gui/CanvasView.h" // For ActiveTool enum

namespace gui {

class Toolbar {
public:
    void Render();

    // Get/Set via ActiveTool enum (shared with CanvasView)
    ActiveTool GetActiveTool() const { return static_cast<ActiveTool>(m_SelectedTool); }
    void SetActiveTool(ActiveTool tool) { m_SelectedTool = static_cast<int>(tool); }

    int GetSelectedToolIndex() const { return m_SelectedTool; }
    const char* GetSelectedToolName() const;

    // Static tool options shared across UI
    static ImVec4 ForegroundColor;
    static ImVec4 BackgroundColor;

    static float BrushSize;
    static float BrushHardness;
    static float BrushOpacity;

private:
    int m_SelectedTool = 0; // 0 = Move (ActiveTool::Move)
};

} // namespace gui
