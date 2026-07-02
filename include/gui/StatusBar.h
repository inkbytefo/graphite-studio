#pragma once

#include "imgui.h"

namespace gui {

class StatusBar {
public:
    void Render();

    // Setters called each frame by main loop
    void SetZoom(float zoom) { m_Zoom = zoom; }
    void SetDocumentSize(int w, int h) { m_DocWidth = w; m_DocHeight = h; }
    void SetMousePos(float x, float y) { m_MouseX = x; m_MouseY = y; }
    void SetHasDocument(bool v) { m_HasDocument = v; }

private:
    float m_Zoom = 1.0f;
    int m_DocWidth = 0;
    int m_DocHeight = 0;
    float m_MouseX = 0.0f;
    float m_MouseY = 0.0f;
    bool m_HasDocument = false;
};

} // namespace gui
