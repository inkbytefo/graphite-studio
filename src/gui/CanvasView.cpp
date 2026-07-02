#include "gui/CanvasView.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace gui {

// Helper function to draw a checkerboard background (transparency indicator)
static void DrawCheckerboard(ImDrawList* drawList, ImVec2 imgMin, ImVec2 imgMax, float zoom) {
    float squareSize = 16.0f;
    
    drawList->PushClipRect(imgMin, imgMax, true);
    
    // Light squares
    drawList->AddRectFilled(imgMin, imgMax, IM_COL32(204, 204, 204, 255));
    
    // Dark squares
    ImU32 darkColor = IM_COL32(170, 170, 170, 255);
    for (float y = imgMin.y; y < imgMax.y; y += squareSize) {
        int rowIdx = static_cast<int>((y - imgMin.y) / squareSize);
        bool oddRow = (rowIdx % 2) == 1;
        
        for (float x = imgMin.x; x < imgMax.x; x += squareSize) {
            int colIdx = static_cast<int>((x - imgMin.x) / squareSize);
            bool oddCol = (colIdx % 2) == 1;
            
            if (oddRow ^ oddCol) {
                ImVec2 sqMin = ImVec2(x, y);
                ImVec2 sqMax = ImVec2(std::min(x + squareSize, imgMax.x), std::min(y + squareSize, imgMax.y));
                drawList->AddRectFilled(sqMin, sqMax, darkColor);
            }
        }
    }
    
    drawList->PopClipRect();
}

CanvasView::CanvasView()
    : m_FboId(0), m_FboTextureId(0), m_ImageTextureId(0),
      m_ImageLoaded(false), m_ImageWidth(0), m_ImageHeight(0),
      m_Zoom(1.0f), m_PanOffset(0.0f, 0.0f), m_IsPanning(false),
      m_LastMousePos(0.0f, 0.0f), m_MouseImageX(0.0f), m_MouseImageY(0.0f),
      m_LastCanvasSize(0.0f, 0.0f), m_NeedsFitToWindow(false) {}

CanvasView::~CanvasView() {
    CleanupFbo();
}

void CanvasView::Init() {
    // Initial OpenGL configurations
}

void CanvasView::CleanupFbo() {
    if (m_FboId != 0) {
        glDeleteFramebuffers(1, &m_FboId);
        m_FboId = 0;
    }
    if (m_FboTextureId != 0) {
        glDeleteTextures(1, &m_FboTextureId);
        m_FboTextureId = 0;
    }
    if (m_ImageTextureId != 0) {
        glDeleteTextures(1, &m_ImageTextureId);
        m_ImageTextureId = 0;
    }
}

void CanvasView::SetupFbo(int width, int height) {
    CleanupFbo();

    // Create GPU Texture
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    // Photoshop pixel rendering style (Nearest Neighbor for crisp pixels)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create FBO (Framebuffer Object)
    glGenFramebuffers(1, &m_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    
    // Bind texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool CanvasView::LoadImageFromFile(const std::string& filepath) {
    if (!m_Image.LoadFromFile(filepath)) {
        return false;
    }

    m_ImageWidth = m_Image.GetWidth();
    m_ImageHeight = m_Image.GetHeight();
    m_ImageLoaded = true;

    // Set up GPU Framebuffer and Texture
    SetupFbo(m_ImageWidth, m_ImageHeight);

    // Upload raw pixel data to GPU FBO Texture
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_ImageWidth, m_ImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_Image.GetPixels());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Schedule fit-to-window on next render (when we know canvas size)
    m_NeedsFitToWindow = true;
    return true;
}

void CanvasView::ResetView() {
    // Fit image to the current canvas panel size
    if (m_ImageLoaded && m_LastCanvasSize.x > 0 && m_LastCanvasSize.y > 0) {
        float scaleX = m_LastCanvasSize.x / static_cast<float>(m_ImageWidth);
        float scaleY = m_LastCanvasSize.y / static_cast<float>(m_ImageHeight);
        
        // Use the smaller scale to fit completely, with a small margin
        m_Zoom = std::min(scaleX, scaleY) * 0.92f;
        
        // Clamp to Photoshop zoom boundaries
        m_Zoom = std::clamp(m_Zoom, 0.05f, 64.0f);
    } else {
        m_Zoom = 1.0f;
    }
    
    m_PanOffset = ImVec2(0.0f, 0.0f);
}

void CanvasView::HandleInputs(ImVec2 canvasCenter, ImVec2 canvasSize) {
    ImGuiIO& io = ImGui::GetIO();
    bool isHovered = ImGui::IsWindowHovered();

    // 1. Pan (Middle mouse click or Space + Left click drag)
    bool spacePressed = ImGui::IsKeyDown(ImGuiKey_Space);
    bool middleMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    bool leftMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    bool shouldPan = isHovered && (middleMouseDown || (spacePressed && leftMouseDown));

    if (shouldPan) {
        if (!m_IsPanning) {
            m_IsPanning = true;
            m_LastMousePos = io.MousePos;
        }
        ImVec2 delta = ImVec2(io.MousePos.x - m_LastMousePos.x, io.MousePos.y - m_LastMousePos.y);
        m_PanOffset.x += delta.x;
        m_PanOffset.y += delta.y;
        m_LastMousePos = io.MousePos;
    } else {
        m_IsPanning = false;
    }

    // Set cursors like Photoshop
    if (isHovered) {
        if (spacePressed) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
    }

    // 2. Zoom to cursor (Mouse Wheel)
    if (isHovered && io.MouseWheel != 0.0f) {
        float prevZoom = m_Zoom;
        float zoomFactor = 1.15f;
        
        if (io.MouseWheel > 0.0f) {
            m_Zoom *= zoomFactor;
        } else {
            m_Zoom /= zoomFactor;
        }

        // Photoshop zoom boundaries (5% - 6400%)
        m_Zoom = std::clamp(m_Zoom, 0.05f, 64.0f);

        // Zoom centered on cursor position
        ImVec2 mousePos = io.MousePos;
        ImVec2 mouseRelToImg = ImVec2(mousePos.x - (canvasCenter.x + m_PanOffset.x),
                                      mousePos.y - (canvasCenter.y + m_PanOffset.y));

        m_PanOffset.x = mousePos.x - canvasCenter.x - (mouseRelToImg.x / prevZoom) * m_Zoom;
        m_PanOffset.y = mousePos.y - canvasCenter.y - (mouseRelToImg.y / prevZoom) * m_Zoom;
    }

    // 3. Track mouse position in image coordinates for StatusBar
    if (isHovered && m_ImageLoaded) {
        ImVec2 imgOrigin = ImVec2(canvasCenter.x + m_PanOffset.x - (m_ImageWidth * m_Zoom) / 2.0f,
                                  canvasCenter.y + m_PanOffset.y - (m_ImageHeight * m_Zoom) / 2.0f);
        m_MouseImageX = (io.MousePos.x - imgOrigin.x) / m_Zoom;
        m_MouseImageY = (io.MousePos.y - imgOrigin.y) / m_Zoom;
        
        // Clamp to image bounds
        m_MouseImageX = std::clamp(m_MouseImageX, 0.0f, static_cast<float>(m_ImageWidth));
        m_MouseImageY = std::clamp(m_MouseImageY, 0.0f, static_cast<float>(m_ImageHeight));
    }
}

void CanvasView::DrawPixelGrid(ImDrawList* drawList, ImVec2 imgMin, ImVec2 imgMax) {
    ImU32 gridColor = IM_COL32(90, 90, 90, 180);
    
    // Vertical grid lines
    for (int x = 1; x < m_ImageWidth; ++x) {
        float posX = imgMin.x + x * m_Zoom;
        drawList->AddLine(ImVec2(posX, imgMin.y), ImVec2(posX, imgMax.y), gridColor);
    }

    // Horizontal grid lines
    for (int y = 1; y < m_ImageHeight; ++y) {
        float posY = imgMin.y + y * m_Zoom;
        drawList->AddLine(ImVec2(imgMin.x, posY), ImVec2(imgMax.x, posY), gridColor);
    }
}

void CanvasView::Render() {
    // Photoshop canvas background color
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.118f, 0.118f, 0.118f, 1.0f)); // #1e1e1e - dark canvas bg
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.118f, 0.118f, 0.118f, 1.0f));
    
    ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    ImVec2 panelPos = ImGui::GetWindowPos();
    ImVec2 cursorStart = ImGui::GetCursorStartPos();
    
    // Track canvas size for fit-to-window
    m_LastCanvasSize = panelSize;
    
    // Calculate the physical center of the ImGui canvas window
    ImVec2 canvasCenter = ImVec2(panelPos.x + cursorStart.x + panelSize.x / 2.0f,
                                 panelPos.y + cursorStart.y + panelSize.y / 2.0f);

    // Execute deferred fit-to-window after we know canvas size
    if (m_NeedsFitToWindow && m_ImageLoaded) {
        ResetView();
        m_NeedsFitToWindow = false;
    }

    if (m_ImageLoaded) {
        // Track Pan & Zoom actions
        HandleInputs(canvasCenter, panelSize);

        // Compute current image boundaries on the screen
        ImVec2 imgMin = ImVec2(canvasCenter.x + m_PanOffset.x - (m_ImageWidth * m_Zoom) / 2.0f,
                               canvasCenter.y + m_PanOffset.y - (m_ImageHeight * m_Zoom) / 2.0f);
        ImVec2 imgMax = ImVec2(imgMin.x + m_ImageWidth * m_Zoom,
                               imgMin.y + m_ImageHeight * m_Zoom);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 1. Draw drop shadow behind the image (like Photoshop)
        ImVec2 shadowOffset = ImVec2(3.0f, 3.0f);
        drawList->AddRectFilled(
            ImVec2(imgMin.x + shadowOffset.x, imgMin.y + shadowOffset.y),
            ImVec2(imgMax.x + shadowOffset.x, imgMax.y + shadowOffset.y),
            IM_COL32(0, 0, 0, 80)
        );

        // 2. Draw transparency checkerboard pattern
        DrawCheckerboard(drawList, imgMin, imgMax, m_Zoom);

        // 3. Render the actual GPU FBO Texture containing image pixels
        drawList->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(m_FboTextureId)), imgMin, imgMax);

        // 4. Draw grid overlay if zoomed >= 800%
        if (m_Zoom >= 8.0f) {
            DrawPixelGrid(drawList, imgMin, imgMax);
        }

        // 5. Draw image border outline
        drawList->AddRect(imgMin, imgMax, IM_COL32(60, 60, 60, 200));
    } else {
        // Centered helper message when no image is loaded
        const char* msg = "Drag an image here or use File > Open";
        ImVec2 textSize = ImGui::CalcTextSize(msg);
        ImVec2 textPos = ImVec2(canvasCenter.x - textSize.x / 2.0f, canvasCenter.y - textSize.y / 2.0f);
        
        ImGui::SetCursorScreenPos(textPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.400f, 0.400f, 0.400f, 1.0f));
        ImGui::TextUnformatted(msg);
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

} // namespace gui
