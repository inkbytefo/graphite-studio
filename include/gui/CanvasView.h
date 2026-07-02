#pragma once

#include <glad/gl.h>
#include "imgui.h"
#include "core/Image.h"
#include <string>

namespace gui {

// Photoshop standard zoom levels (percentage / 100)
static const float kZoomLevels[] = {
    0.03125f,  // 3.125%
    0.04f,     // 4%
    0.05f,     // 5%
    0.0625f,   // 6.25%
    0.0833f,   // 8.33%
    0.125f,    // 12.5%
    0.1667f,   // 16.67%
    0.25f,     // 25%
    0.3333f,   // 33.33%
    0.5f,      // 50%
    0.6667f,   // 66.67%
    1.0f,      // 100%
    2.0f,      // 200%
    3.0f,      // 300%
    4.0f,      // 400%
    5.0f,      // 500%
    6.0f,      // 600%
    8.0f,      // 800%
    12.0f,     // 1200%
    16.0f,     // 1600%
    32.0f,     // 3200%
    64.0f,     // 6400%
    128.0f,    // 12800%
};
static const int kZoomLevelCount = sizeof(kZoomLevels) / sizeof(kZoomLevels[0]);

// Active tool identifiers (shared with Toolbar)
enum class ActiveTool {
    Move = 0,
    MarqueeSelect,
    Lasso,
    MagicWand,
    Crop,
    Eyedropper,
    HealingBrush,
    Brush,
    CloneStamp,
    HistoryBrush,
    Eraser,
    Gradient,
    Dodge,
    Pen,
    Text,
    PathSelection,
    Shape,
    Hand,
    Zoom,
    Count
};

class CanvasView {
public:
    CanvasView();
    ~CanvasView();

    // Initialize OpenGL state
    void Init();

    // Load image into canvas
    bool LoadImageFromFile(const std::string& filepath);

    // Render the canvas window
    void Render();

    // Reset pan and zoom to fit image in window
    void ResetView();

    // Zoom control API (called from main.cpp shortcuts)
    void ZoomIn();          // Ctrl + +  : Next zoom step
    void ZoomOut();         // Ctrl + -  : Previous zoom step
    void ZoomToActual();    // Ctrl + 1  : Zoom to 100%

    // Pixel grid toggle
    void TogglePixelGrid() { m_ShowPixelGrid = !m_ShowPixelGrid; }
    bool IsPixelGridVisible() const { return m_ShowPixelGrid; }

    // Active tool (set by Toolbar or keyboard shortcut)
    void SetActiveTool(ActiveTool tool) { m_ActiveTool = tool; }
    ActiveTool GetActiveTool() const { return m_ActiveTool; }

    // Accessors
    bool IsImageLoaded() const { return m_ImageLoaded; }
    int GetImageWidth() const { return m_ImageWidth; }
    int GetImageHeight() const { return m_ImageHeight; }
    float GetZoom() const { return m_Zoom; }

    // Get mouse position relative to the image (in image pixel coords)
    void GetMouseImageCoords(float& outX, float& outY) const { outX = m_MouseImageX; outY = m_MouseImageY; }

private:
    void SetupFbo(int width, int height);
    void UpdateFbo();
    void CleanupFbo();
    void HandleInputs(ImVec2 canvasCenter, ImVec2 canvasSize);
    void DrawPixelGrid(ImDrawList* drawList, ImVec2 imgMin, ImVec2 imgMax);

    // Zoom helpers
    void ZoomToLevel(float newZoom, ImVec2 zoomCenter, ImVec2 canvasCenter);
    int FindNearestZoomIndex(float zoom) const;

    // OpenGL Handles
    GLuint m_FboId;
    GLuint m_FboTextureId;
    GLuint m_ImageTextureId;

    // Image properties
    core::Image m_Image;
    bool m_ImageLoaded;
    int m_ImageWidth;
    int m_ImageHeight;

    // Viewport pan & zoom properties
    float m_Zoom;
    ImVec2 m_PanOffset;
    bool m_IsPanning;
    ImVec2 m_LastMousePos;

    // Scrubby zoom state
    bool m_IsScrubbyZooming;
    float m_ScrubbyStartX;
    float m_ScrubbyStartZoom;

    // Active tool
    ActiveTool m_ActiveTool;

    // Pixel grid visibility
    bool m_ShowPixelGrid;

    // For status bar: mouse position in image coordinates
    float m_MouseImageX;
    float m_MouseImageY;

    // Track canvas panel size for fit-to-window
    ImVec2 m_LastCanvasSize;
    bool m_NeedsFitToWindow;
};

} // namespace gui
