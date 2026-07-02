#pragma once

#include "imgui.h"
#include "gui/CanvasView.h"

namespace gui {

class LayersPanel {
public:
    void Render(CanvasView& canvasView);
};

} // namespace gui
