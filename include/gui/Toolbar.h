#pragma once

#include "imgui.h"

namespace gui {

class Toolbar {
public:
    void Render();

    int GetSelectedTool() const { return m_SelectedTool; }
    const char* GetSelectedToolName() const;

private:
    int m_SelectedTool = 0; // 0 = Move
};

} // namespace gui
