#pragma once

#include "imgui.h"

namespace gui {

// Initialize the Photoshop 2026 dark theme
void ApplyPhotoshopTheme();

// Load professional fonts (Segoe UI on Windows, fallback otherwise)
void InitFonts(ImGuiIO& io);

} // namespace gui
