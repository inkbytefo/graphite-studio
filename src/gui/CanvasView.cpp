#include "gui/CanvasView.h"
#include "gui/Toolbar.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace gui {

// Helper function to draw a checkerboard background (transparency indicator)
// Photoshop behavior: checkerboard squares stay fixed size on screen regardless of zoom
static void DrawCheckerboard(ImDrawList* drawList, ImVec2 imgMin, ImVec2 imgMax) {
    const float squareSize = 12.0f; // Fixed screen-space size (Photoshop-style)

    drawList->PushClipRect(imgMin, imgMax, true);

    // Light squares base
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
    : m_Zoom(1.0f), m_PanOffset(0.0f, 0.0f), m_IsPanning(false),
      m_LastMousePos(0.0f, 0.0f),
      m_IsScrubbyZooming(false), m_ScrubbyStartX(0.0f), m_ScrubbyStartZoom(1.0f),
      m_ActiveTool(ActiveTool::Move),
      m_ShowPixelGrid(true),
      m_MouseImageX(0.0f), m_MouseImageY(0.0f),
      m_LastCanvasSize(0.0f, 0.0f), m_NeedsFitToWindow(false) {}

CanvasView::~CanvasView() {
    CleanupFbo();
}

void CanvasView::Init() {
    m_Compositor.Init();
}

void CanvasView::CleanupFbo() {
    m_Fbo.Destroy();
    m_FboTexture.Destroy();
}

void CanvasView::SetupFbo(int width, int height) {
    std::cout << "[CanvasView] SetupFbo called with dimensions: " << width << "x" << height << std::endl;
    CleanupFbo();

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[CanvasView] OpenGL error before SetupFbo: " << err << std::endl;
    }

    // Create FBO Texture using modern DSA and RAII
    m_FboTexture.Create(GL_TEXTURE_2D);
    m_FboTexture.AllocateStorage2D(1, GL_RGBA8, width, height);

    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[CanvasView] glTextureStorage2D failed with error: " << err << std::endl;
    }

    // Photoshop pixel rendering style (Nearest Neighbor for crisp pixels) using DSA/RAII
    m_FboTexture.SetParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_FboTexture.SetParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_FboTexture.SetParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_FboTexture.SetParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create FBO (Framebuffer Object) using modern DSA and RAII
    m_Fbo.Create();

    // Bind texture to FBO using DSA/RAII
    m_Fbo.AttachTexture(GL_COLOR_ATTACHMENT0, m_FboTexture, 0);

    if (!m_Fbo.CheckStatus(GL_FRAMEBUFFER)) {
        std::cerr << "[CanvasView] Framebuffer is not complete!" << std::endl;
    } else {
        std::cout << "[CanvasView] Framebuffer " << m_Fbo.GetId() << " created successfully (Complete) via DSA/RAII." << std::endl;
    }

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool CanvasView::LoadImageFromFile(const std::string& filepath) {
    if (!m_Document.LoadFromFile(filepath)) {
        return false;
    }

    // Set up GPU Framebuffer (FBO) for target rendering
    SetupFbo(m_Document.GetWidth(), m_Document.GetHeight());
    
    m_NeedsFitToWindow = true;
    return true;
}

bool CanvasView::SaveCompositeToFile(const std::string& filepath) {
    return m_Document.SaveCompositeToFile(filepath, m_FboTexture.GetId());
}

bool CanvasView::CreateNewDocument(const std::string& name, int width, int height, ImVec4 bgColor) {
    if (!m_Document.CreateNew(name, width, height, bgColor)) {
        return false;
    }

    // Set up GPU Framebuffer (FBO) for target rendering
    SetupFbo(width, height);

    m_NeedsFitToWindow = true;
    return true;
}

// ─── Zoom Helpers ────────────────────────────────────────────────────────────

int CanvasView::FindNearestZoomIndex(float zoom) const {
    int bestIdx = 0;
    float bestDist = std::abs(std::log(zoom) - std::log(kZoomLevels[0]));
    for (int i = 1; i < kZoomLevels.size(); ++i) {
        float dist = std::abs(std::log(zoom) - std::log(kZoomLevels[i]));
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void CanvasView::ZoomToLevel(float newZoom, ImVec2 zoomCenter, ImVec2 canvasCenter) {
    float prevZoom = m_Zoom;
    m_Zoom = std::clamp(newZoom, kZoomLevels[0], kZoomLevels[kZoomLevels.size() - 1]);

    // Adjust pan offset so the zoom center stays fixed on screen
    ImVec2 mouseRelToImg = ImVec2(zoomCenter.x - (canvasCenter.x + m_PanOffset.x),
                                  zoomCenter.y - (canvasCenter.y + m_PanOffset.y));

    m_PanOffset.x = zoomCenter.x - canvasCenter.x - (mouseRelToImg.x / prevZoom) * m_Zoom;
    m_PanOffset.y = zoomCenter.y - canvasCenter.y - (mouseRelToImg.y / prevZoom) * m_Zoom;
}

void CanvasView::ZoomIn() {
    int idx = FindNearestZoomIndex(m_Zoom);
    if (idx < kZoomLevels.size() - 1) {
        // If we're already at or very close to this level, go to the next
        if (std::abs(m_Zoom - kZoomLevels[idx]) < 0.001f || m_Zoom >= kZoomLevels[idx]) {
            idx++;
        }
        if (idx < kZoomLevels.size()) {
            // Zoom centered on canvas center
            ImVec2 canvasCenter = ImVec2(m_LastCanvasSize.x / 2.0f, m_LastCanvasSize.y / 2.0f);
            float prevZoom = m_Zoom;
            m_Zoom = kZoomLevels[idx];
            // Keep pan offset centered (no shift)
        }
    }
}

void CanvasView::ZoomOut() {
    int idx = FindNearestZoomIndex(m_Zoom);
    if (idx > 0) {
        // If we're already at or very close to this level, go to the previous
        if (std::abs(m_Zoom - kZoomLevels[idx]) < 0.001f || m_Zoom <= kZoomLevels[idx]) {
            idx--;
        }
        if (idx >= 0) {
            m_Zoom = kZoomLevels[idx];
        }
    }
}

void CanvasView::ZoomToActual() {
    m_Zoom = 1.0f;
    m_PanOffset = ImVec2(0.0f, 0.0f);
}

void CanvasView::ResetView() {
    // Fit image to the current canvas panel size
    if (m_Document.IsLoaded() && m_LastCanvasSize.x > 0 && m_LastCanvasSize.y > 0) {
        float scaleX = m_LastCanvasSize.x / static_cast<float>(m_Document.GetWidth());
        float scaleY = m_LastCanvasSize.y / static_cast<float>(m_Document.GetHeight());

        // Use the smaller scale to fit completely, with a small margin
        m_Zoom = std::min(scaleX, scaleY) * 0.92f;

        // Clamp to Photoshop zoom boundaries
        m_Zoom = std::clamp(m_Zoom, kZoomLevels[0], kZoomLevels[kZoomLevels.size() - 1]);
    } else {
        m_Zoom = 1.0f;
    }

    m_PanOffset = ImVec2(0.0f, 0.0f);
}

// ─── Input Handling ──────────────────────────────────────────────────────────

void CanvasView::HandleInputs(ImVec2 canvasCenter, ImVec2 canvasSize) {
    ImGuiIO& io = ImGui::GetIO();
    bool isHovered = ImGui::IsWindowHovered();

    // ── 1. Pan: Middle mouse, Space+Left, or Hand tool + Left ──────────────
    bool spacePressed = ImGui::IsKeyDown(ImGuiKey_Space);
    bool middleMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    bool leftMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    bool isHandTool = (m_ActiveTool == ActiveTool::Hand);

    bool shouldPan = isHovered && (middleMouseDown || (spacePressed && leftMouseDown) || (isHandTool && leftMouseDown));

    if (shouldPan && !m_IsScrubbyZooming) {
        if (!m_IsPanning) {
            m_IsPanning = true;
            m_LastMousePos = io.MousePos;
        }
        ImVec2 delta = ImVec2(io.MousePos.x - m_LastMousePos.x, io.MousePos.y - m_LastMousePos.y);
        m_PanOffset.x += delta.x;
        m_PanOffset.y += delta.y;
        m_LastMousePos = io.MousePos;
    } else if (!shouldPan) {
        m_IsPanning = false;
    }

    // ── 2. Scrubby Zoom: Zoom tool + Left click drag ──────────────────────
    bool isZoomTool = (m_ActiveTool == ActiveTool::Zoom);
    if (isHovered && isZoomTool && !m_IsPanning) {
        if (leftMouseDown) {
            if (!m_IsScrubbyZooming) {
                m_IsScrubbyZooming = true;
                m_ScrubbyStartX = io.MousePos.x;
                m_ScrubbyStartZoom = m_Zoom;
            }
            // Scrubby: drag right = zoom in, drag left = zoom out
            float dragDelta = io.MousePos.x - m_ScrubbyStartX;
            float sensitivity = 0.005f; // Pixels per zoom factor
            float factor = std::exp(dragDelta * sensitivity);
            float newZoom = m_ScrubbyStartZoom * factor;
            newZoom = std::clamp(newZoom, kZoomLevels[0], kZoomLevels[kZoomLevels.size() - 1]);

            // Zoom centered on scrubby start position
            ZoomToLevel(newZoom, ImVec2(m_ScrubbyStartX, io.MousePos.y), canvasCenter);
        } else {
            m_IsScrubbyZooming = false;
        }
    } else {
        m_IsScrubbyZooming = false;
    }

    // ── 3. Cursor appearance (Photoshop-accurate) ──────────────────────────
    if (isHovered) {
        if (m_IsPanning || spacePressed || isHandTool) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); // Open hand / grab
        } else if (isZoomTool) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); // Magnifying glass approximation
        }
    }

    // ── 4. Zoom to cursor (Mouse Wheel) ────────────────────────────────────
    if (isHovered && io.MouseWheel != 0.0f && !m_IsScrubbyZooming) {
        int idx = FindNearestZoomIndex(m_Zoom);

        if (io.MouseWheel > 0.0f) {
            // Zoom in: go to next step
            if (m_Zoom >= kZoomLevels[idx] - 0.001f && idx < kZoomLevels.size() - 1) {
                idx++;
            }
        } else {
            // Zoom out: go to previous step
            if (m_Zoom <= kZoomLevels[idx] + 0.001f && idx > 0) {
                idx--;
            }
        }

        ZoomToLevel(kZoomLevels[idx], io.MousePos, canvasCenter);
    }

    // ── 5. Track mouse position in image coordinates for StatusBar ─────────
    if (isHovered && m_Document.IsLoaded()) {
        ImVec2 imgOrigin = ImVec2(canvasCenter.x + m_PanOffset.x - (m_Document.GetWidth() * m_Zoom) / 2.0f,
                                  canvasCenter.y + m_PanOffset.y - (m_Document.GetHeight() * m_Zoom) / 2.0f);
        m_MouseImageX = (io.MousePos.x - imgOrigin.x) / m_Zoom;
        m_MouseImageY = (io.MousePos.y - imgOrigin.y) / m_Zoom;

        // Clamp to image bounds
        m_MouseImageX = std::clamp(m_MouseImageX, 0.0f, static_cast<float>(m_Document.GetWidth()));
        m_MouseImageY = std::clamp(m_MouseImageY, 0.0f, static_cast<float>(m_Document.GetHeight()));
    }
}

// ─── Pixel Grid ──────────────────────────────────────────────────────────────

void CanvasView::DrawPixelGrid(ImDrawList* drawList, ImVec2 imgMin, ImVec2 imgMax) {
    // Only draw within the visible canvas region to avoid performance issues
    ImVec2 clipMin = drawList->GetClipRectMin();
    ImVec2 clipMax = drawList->GetClipRectMax();

    float startX = std::max(imgMin.x, clipMin.x);
    float endX   = std::min(imgMax.x, clipMax.x);
    float startY = std::max(imgMin.y, clipMin.y);
    float endY   = std::min(imgMax.y, clipMax.y);

    // Calculate the range of pixels visible
    int pixStartX = std::max(0, static_cast<int>((startX - imgMin.x) / m_Zoom));
    int pixEndX   = std::min(m_Document.GetWidth(), static_cast<int>((endX - imgMin.x) / m_Zoom) + 1);
    int pixStartY = std::max(0, static_cast<int>((startY - imgMin.y) / m_Zoom));
    int pixEndY   = std::min(m_Document.GetHeight(), static_cast<int>((endY - imgMin.y) / m_Zoom) + 1);

    ImU32 gridColor = IM_COL32(128, 128, 128, 60);

    // Vertical grid lines
    for (int x = pixStartX + 1; x < pixEndX; ++x) {
        float posX = imgMin.x + x * m_Zoom;
        drawList->AddLine(ImVec2(posX, startY), ImVec2(posX, endY), gridColor);
    }

    // Horizontal grid lines
    for (int y = pixStartY + 1; y < pixEndY; ++y) {
        float posY = imgMin.y + y * m_Zoom;
        drawList->AddLine(ImVec2(startX, posY), ImVec2(endX, posY), gridColor);
    }
}

// ─── Render ──────────────────────────────────────────────────────────────────

void CanvasView::Render() {
    // Photoshop canvas background color
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.118f, 0.118f, 0.118f, 1.0f)); // #1e1e1e
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
    if (m_NeedsFitToWindow && m_Document.IsLoaded()) {
        ResetView();
        m_NeedsFitToWindow = false;
    }

    if (m_Document.IsLoaded()) {
        // 0. Composite all layers on GPU onto our FBO
        m_Compositor.Composite(m_Document.GetLayerStack(), m_Fbo.GetId(), m_Document.GetWidth(), m_Document.GetHeight());

        // Track Pan & Zoom actions
        HandleInputs(canvasCenter, panelSize);

        // Track Drawing actions
        HandleDrawing(canvasCenter, panelSize);

        // Compute current image boundaries on the screen
        ImVec2 imgMin = ImVec2(canvasCenter.x + m_PanOffset.x - (m_Document.GetWidth() * m_Zoom) / 2.0f,
                               canvasCenter.y + m_PanOffset.y - (m_Document.GetHeight() * m_Zoom) / 2.0f);
        ImVec2 imgMax = ImVec2(imgMin.x + m_Document.GetWidth() * m_Zoom,
                               imgMin.y + m_Document.GetHeight() * m_Zoom);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 1. Draw drop shadow behind the image (Photoshop style)
        float shadowSize = std::max(3.0f, m_Zoom * 0.5f);
        shadowSize = std::min(shadowSize, 8.0f); // Cap shadow to avoid huge offsets
        ImVec2 shadowOffset = ImVec2(shadowSize, shadowSize);
        drawList->AddRectFilled(
            ImVec2(imgMin.x + shadowOffset.x, imgMin.y + shadowOffset.y),
            ImVec2(imgMax.x + shadowOffset.x, imgMax.y + shadowOffset.y),
            IM_COL32(0, 0, 0, 80)
        );

        // 2. Draw transparency checkerboard pattern (fixed screen-space size)
        DrawCheckerboard(drawList, imgMin, imgMax);

        // 3. Render the actual GPU FBO Texture containing image pixels
        drawList->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(m_FboTexture.GetId())), imgMin, imgMax);

        // 4. Draw pixel grid overlay if enabled and zoom >= 500%
        if (m_ShowPixelGrid && m_Zoom >= 5.0f) {
            DrawPixelGrid(drawList, imgMin, imgMax);
        }

        // 5. Draw image border outline
        drawList->AddRect(imgMin, imgMax, IM_COL32(60, 60, 60, 200));

        // 6. Draw custom brush circle preview if Brush or Eraser is hovered
        bool isHovered = ImGui::IsWindowHovered();
        bool isBrush = (m_ActiveTool == ActiveTool::Brush);
        bool isEraser = (m_ActiveTool == ActiveTool::Eraser);
        if (isHovered && (isBrush || isEraser) && !m_IsPanning && !m_IsScrubbyZooming) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None); // Hide default mouse cursor
            float screenRadius = (Toolbar::BrushSize * m_Zoom) / 2.0f;
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            
            // Draw dual-color circle to stay visible on any background
            drawList->AddCircle(mousePos, screenRadius, IM_COL32(230, 230, 230, 255), 32, 1.0f);
            drawList->AddCircle(mousePos, screenRadius + 0.5f, IM_COL32(20, 20, 20, 150), 32, 1.0f);
        }
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
void CanvasView::HandleDrawing(ImVec2 canvasCenter, ImVec2 canvasSize) {
    ImGuiIO& io = ImGui::GetIO();
    bool isHovered = ImGui::IsWindowHovered();

    bool isBrush = (m_ActiveTool == ActiveTool::Brush);
    bool isEraser = (m_ActiveTool == ActiveTool::Eraser);
    
    // Only draw if Brush or Eraser tool is selected, and we aren't panning or scrubby zooming
    if (m_Document.IsLoaded() && (isBrush || isEraser) && !m_IsPanning && !m_IsScrubbyZooming) {
        bool leftMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
        
        // Map mouse position to image coordinates
        ImVec2 imgOrigin = ImVec2(canvasCenter.x + m_PanOffset.x - (m_Document.GetWidth() * m_Zoom) / 2.0f,
                                  canvasCenter.y + m_PanOffset.y - (m_Document.GetHeight() * m_Zoom) / 2.0f);
        
        ImVec2 currentMouseImgCoords = ImVec2(
            (io.MousePos.x - imgOrigin.x) / m_Zoom,
            (io.MousePos.y - imgOrigin.y) / m_Zoom
        );

        if (leftMouseDown && isHovered) {
            if (!m_BrushEngine.IsActive()) {
                // Save history state BEFORE drawing starts
                m_Document.GetHistory().RecordState(m_Document.GetLayerStack(), isBrush ? "Brush Tool" : "Eraser Tool");

                core::BrushParams params;
                params.size = Toolbar::BrushSize;
                params.hardness = Toolbar::BrushHardness;
                params.opacity = Toolbar::BrushOpacity;
                params.color = Toolbar::ForegroundColor;
                params.isEraser = isEraser;

                m_BrushEngine.BeginStroke(m_Document.GetLayerStack().GetSelectedLayer(), params);
            }
            m_BrushEngine.ContinueStroke(currentMouseImgCoords);
        } else {
            if (m_BrushEngine.IsActive()) {
                m_BrushEngine.EndStroke();
            }
        }
    } else {
        if (m_BrushEngine.IsActive()) {
            m_BrushEngine.EndStroke();
        }
    }
}

} // namespace gui
