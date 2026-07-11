#pragma once

#include "imgui.h"
#include "core/Layer.h"
#include <vector>

namespace core {

struct BrushParams {
    float size = 20.0f;
    float hardness = 0.5f;
    float opacity = 1.0f;
    ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    bool isEraser = false;
};

class BrushEngine {
public:
    // Call when the user presses the mouse button to start drawing
    void BeginStroke(Layer* target, const BrushParams& params);

    // Call each frame while the user is dragging the mouse
    void ContinueStroke(ImVec2 currentPos);

    // Call when the user releases the mouse button
    void EndStroke();

    bool IsActive() const { return m_IsActive; }

private:
    void PaintSegment(ImVec2 from, ImVec2 to);

    Layer* m_Target = nullptr;
    BrushParams m_Params;
    bool m_IsActive = false;
    ImVec2 m_LastPos = {0, 0};
};

} // namespace core
