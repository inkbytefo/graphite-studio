#pragma once
#include "imgui.h"
#include "gui/CanvasView.h"

namespace gui {

class OptionsBar {
public:
    void Render(CanvasView& canvasView);
};

} // namespace gui
