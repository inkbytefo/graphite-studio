#pragma once

#include "imgui.h"

namespace gui {

// Setup the default Photoshop-like docking layout on first launch or when forced
void SetupDefaultLayout(ImGuiID dockspace_id, bool force = false);

} // namespace gui
