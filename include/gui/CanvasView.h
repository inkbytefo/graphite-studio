#pragma once

#include <glad/gl.h>
#include "imgui.h"
#include "core/Image.h"
#include <string>

namespace gui {

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

    // For status bar: mouse position in image coordinates
    float m_MouseImageX;
    float m_MouseImageY;

    // Track canvas panel size for fit-to-window
    ImVec2 m_LastCanvasSize;
    bool m_NeedsFitToWindow;
};

} // namespace gui
