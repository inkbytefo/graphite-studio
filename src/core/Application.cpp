#include "core/Application.h"
#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "platform/FileDialogs.h"
#include "gui/Theme.h"
#include "gui/Workspace.h"
#include "gui/CanvasView.h"
#include "gui/Toolbar.h"
#include "gui/StatusBar.h"
#include "gui/PropertiesPanel.h"
#include "gui/LayersPanel.h"
#include "gui/OptionsBar.h"
#include "gui/HistoryPanel.h"
#include "gui/ColorPanel.h"
#include "core/Layer.h"
#include "core/LayerStack.h"
#include "core/DocumentCommands.h"

namespace core {

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

Application::Application() {}

Application::~Application() {
    Shutdown();
}

bool Application::Init() {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window borderless
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    m_Window = glfwCreateWindow(1600, 900, "Graphite Studio 2026", nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Setup Dear ImGui style and fonts
    gui::ApplyPhotoshopTheme();
    gui::InitFonts(io);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize UI Panels and Views
    m_CanvasView = new gui::CanvasView();
    m_CanvasView->Init();

    m_Toolbar = new gui::Toolbar();
    m_StatusBar = new gui::StatusBar();
    m_PropertiesPanel = new gui::PropertiesPanel();
    m_LayersPanel = new gui::LayersPanel();
    m_OptionsBar = new gui::OptionsBar();
    m_HistoryPanel = new gui::HistoryPanel();
    m_ColorPanel = new gui::ColorPanel();

    return true;
}

void Application::Run() {
    ImGuiIO& io = ImGui::GetIO();

    while (!glfwWindowShouldClose(m_Window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Handle custom title bar dragging
        static bool isDragging = false;
        static double dragStartX = 0.0;
        static double dragStartY = 0.0;
        static int dragStartWinX = 0;
        static int dragStartWinY = 0;

        if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(m_Window, &mouseX, &mouseY);
            int winWidth, winHeight;
            glfwGetWindowSize(m_Window, &winWidth, &winHeight);
            int winX, winY;
            glfwGetWindowPos(m_Window, &winX, &winY);
            double screenMouseX = winX + mouseX;
            double screenMouseY = winY + mouseY;

            if (!isDragging) {
                if (mouseY >= 0.0 && mouseY <= 30.0 && mouseX < (winWidth - 140.0)) {
                    static double lastClickTime = 0.0;
                    double currentTime = glfwGetTime();
                    if (currentTime - lastClickTime < 0.3) {
                        if (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED)) {
                            glfwRestoreWindow(m_Window);
                        } else {
                            glfwMaximizeWindow(m_Window);
                        }
                        isDragging = false;
                    } else if (!glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED)) {
                        isDragging = true;
                        dragStartX = screenMouseX;
                        dragStartY = screenMouseY;
                        dragStartWinX = winX;
                        dragStartWinY = winY;
                    }
                    lastClickTime = currentTime;
                }
            } else {
                int deltaX = static_cast<int>(screenMouseX - dragStartX);
                int deltaY = static_cast<int>(screenMouseY - dragStartY);
                glfwSetWindowPos(m_Window, dragStartWinX + deltaX, dragStartWinY + deltaY);
            }
        } else {
            isDragging = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render custom title bar
        RenderTitleBar();

        // Handle Global Shortcuts
        HandleGlobalShortcuts();

        // Set up the DockSpace
        SetupDockSpace(m_ResetLayout);

        // Render panels and views
        m_Toolbar->Render();

        // Sync active tool between Toolbar and CanvasView (bidirectional)
        m_CanvasView->SetActiveTool(m_Toolbar->GetActiveTool());

        m_OptionsBar->Render(*m_CanvasView);
        m_PropertiesPanel->Render(*m_CanvasView);
        m_HistoryPanel->Render(*m_CanvasView);
        m_ColorPanel->Render(*m_CanvasView);
        m_LayersPanel->Render(*m_CanvasView);
        m_CanvasView->Render();

        // Update StatusBar values and render it
        m_StatusBar->SetHasDocument(m_CanvasView->IsImageLoaded());
        if (m_CanvasView->IsImageLoaded()) {
            m_StatusBar->SetZoom(m_CanvasView->GetZoom());
            m_StatusBar->SetDocumentSize(m_CanvasView->GetImageWidth(), m_CanvasView->GetImageHeight());
            float mx, my;
            m_CanvasView->GetMouseImageCoords(mx, my);
            m_StatusBar->SetMousePos(mx, my);
        }
        m_StatusBar->Render();

        // New Document Popup Modal
        if (m_ShowNewDocPopup) {
            ImGui::OpenPopup("New Document");
        }
        RenderNewDocumentPopup();

        // Show Demo Window if checked
        if (m_ShowDemoWindow) {
            ImGui::ShowDemoWindow(&m_ShowDemoWindow);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(m_ClearColor[0] * m_ClearColor[3], m_ClearColor[1] * m_ClearColor[3], m_ClearColor[2] * m_ClearColor[3], m_ClearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows (if multi-viewport enabled)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(m_Window);
    }
}

void Application::HandleGlobalShortcuts() {
    ImGuiIO& io = ImGui::GetIO();

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        std::string filepath = platform::OpenFileDialog();
        if (!filepath.empty()) {
            m_CanvasView->LoadImageFromFile(filepath);
        }
    }
    // New Document: Ctrl+N
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N) && !io.KeyShift) {
        m_ShowNewDocPopup = true;
    }
    // Undo: Ctrl+Z
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
        if (io.KeyShift) {
            m_CanvasView->GetHistoryManager().Redo(m_CanvasView->GetLayerStack());
        } else {
            m_CanvasView->GetHistoryManager().Undo(m_CanvasView->GetLayerStack());
        }
    }
    // Redo: Ctrl+Y
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
        m_CanvasView->GetHistoryManager().Redo(m_CanvasView->GetLayerStack());
    }

    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_Keypad0))) {
        m_CanvasView->ResetView();
    }
    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_1) || ImGui::IsKeyPressed(ImGuiKey_Keypad1))) {
        m_CanvasView->ZoomToActual();
    }
    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) || ImGui::IsKeyPressed(ImGuiKey_Equal))) {
        m_CanvasView->ZoomIn();
    }
    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) || ImGui::IsKeyPressed(ImGuiKey_Minus))) {
        m_CanvasView->ZoomOut();
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Apostrophe)) {
        m_CanvasView->TogglePixelGrid();
    }

    // Layer stack shortcuts (Photoshop-accurate)
    if (m_CanvasView->IsImageLoaded()) {
        core::LayerStack& stack = m_CanvasView->GetLayerStack();
        core::HistoryManager& history = m_CanvasView->GetHistoryManager();
        int selIdx = stack.GetSelectedIndex();

        if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_N)) {
            core::commands::NewLayer(stack, history, m_CanvasView->GetImageWidth(), m_CanvasView->GetImageHeight());
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_J)) {
            core::commands::DuplicateLayer(stack, history, selIdx);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::GetIO().WantTextInput) {
            core::commands::DeleteLayer(stack, history, selIdx);
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E)) {
            core::commands::MergeDown(stack, history, selIdx);
        }
        if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
            core::commands::SelectLayerDown(stack, selIdx);
        }
        if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
            core::commands::SelectLayerUp(stack, selIdx);
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
            core::commands::MoveLayerDown(stack, selIdx);
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
            core::commands::MoveLayerUp(stack, selIdx);
        }
    }
}

void Application::SetupDockSpace(bool& reset_layout) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float statusBarHeight = 24.0f;
    float titleBarHeight = 30.0f;
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    workPos.y += titleBarHeight;
    workSize.y -= (statusBarHeight + titleBarHeight);

    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize(workSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("GraphiteStudioWorkspace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("GraphiteStudioDockSpace");
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

    gui::SetupDefaultLayout(dockspace_id, reset_layout);
    if (reset_layout) {
        reset_layout = false;
    }

    RenderMenuBar(reset_layout);

    ImGui::End();
}

void Application::RenderMenuBar(bool& reset_layout) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New...", "Ctrl+N")) {
                m_ShowNewDocPopup = true;
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                std::string filepath = platform::OpenFileDialog();
                if (!filepath.empty()) {
                    m_CanvasView->LoadImageFromFile(filepath);
                }
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, m_CanvasView->IsImageLoaded())) {
                std::string filepath = platform::SaveFileDialog();
                if (!filepath.empty()) {
                    m_CanvasView->SaveCompositeToFile(filepath);
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(m_Window, true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            bool canUndo = m_CanvasView->GetHistoryManager().CanUndo();
            bool canRedo = m_CanvasView->GetHistoryManager().CanRedo();
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, canUndo)) {
                m_CanvasView->GetHistoryManager().Undo(m_CanvasView->GetLayerStack());
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, canRedo)) {
                m_CanvasView->GetHistoryManager().Redo(m_CanvasView->GetLayerStack());
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Image")) {
            ImGui::MenuItem("Image Size...", nullptr, false, false);
            ImGui::MenuItem("Canvas Size...", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Layer")) {
            bool hasDoc = m_CanvasView->IsImageLoaded();
            core::LayerStack& stack = m_CanvasView->GetLayerStack();
            core::HistoryManager& history = m_CanvasView->GetHistoryManager();
            int selIdx = stack.GetSelectedIndex();

            if (ImGui::MenuItem("New Layer", "Ctrl+Shift+N", false, hasDoc)) {
                core::commands::NewLayer(stack, history, m_CanvasView->GetImageWidth(), m_CanvasView->GetImageHeight());
            }
            if (ImGui::MenuItem("Duplicate Layer", "Ctrl+J", false, hasDoc && selIdx != -1)) {
                core::commands::DuplicateLayer(stack, history, selIdx);
            }
            if (ImGui::MenuItem("Delete Layer", "Delete", false, hasDoc && selIdx != -1 && stack.GetCount() > 1)) {
                core::commands::DeleteLayer(stack, history, selIdx);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Merge Down", "Ctrl+E", false, hasDoc && selIdx > 0)) {
                core::commands::MergeDown(stack, history, selIdx);
            }
            if (ImGui::MenuItem("Merge Visible", "Ctrl+Shift+E", false, hasDoc && stack.GetCount() > 1)) {
                core::commands::MergeVisible(stack, history, m_CanvasView->GetImageWidth(), m_CanvasView->GetImageHeight());
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Filter")) {
            ImGui::MenuItem("Blur", nullptr, false, false);
            ImGui::MenuItem("Sharpen", nullptr, false, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                m_CanvasView->ZoomIn();
            }
            if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                m_CanvasView->ZoomOut();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Fit on Screen", "Ctrl+0")) {
                m_CanvasView->ResetView();
            }
            if (ImGui::MenuItem("Actual Pixels (100%)", "Ctrl+1")) {
                m_CanvasView->ZoomToActual();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Show")) {
                bool pixelGrid = m_CanvasView->IsPixelGridVisible();
                if (ImGui::MenuItem("Pixel Grid", "Ctrl+'", &pixelGrid)) {
                    if (pixelGrid != m_CanvasView->IsPixelGridVisible()) {
                        m_CanvasView->TogglePixelGrid();
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", nullptr, &m_ShowDemoWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Reset Layout")) {
                reset_layout = true;
            }
            ImGui::Separator();
            ImGui::MenuItem("Properties", nullptr, nullptr, true);
            ImGui::MenuItem("Layers", nullptr, nullptr, true);
            ImGui::MenuItem("Toolbar", nullptr, nullptr, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("About Graphite Studio", nullptr, false, false);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Application::RenderNewDocumentPopup() {
    if (ImGui::BeginPopupModal("New Document", &m_ShowNewDocPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create New Document");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4.0f));

        static char docName[64] = "Untitled-1";
        static int selectedPresetIdx = 3; // Default 1080p Full HD
        const char* presets[] = {
            "Custom",
            "A4 Paper (2480 x 3508 px, 300 DPI)",
            "A3 Paper (3508 x 4960 px, 300 DPI)",
            "1080p Full HD (1920 x 1080 px)",
            "4K Ultra HD (3840 x 2160 px)",
            "Social Media Square (1080 x 1080 px)",
            "Web Common (1366 x 768 px)"
        };
        
        static int width = 1920;
        static int height = 1080;
        static int backgroundType = 1; // 0=Transparent, 1=White, 2=Black

        if (ImGui::Combo("Preset Template", &selectedPresetIdx, presets, IM_ARRAYSIZE(presets))) {
            if (selectedPresetIdx == 1) { width = 2480; height = 3508; } // A4
            else if (selectedPresetIdx == 2) { width = 3508; height = 4960; } // A3
            else if (selectedPresetIdx == 3) { width = 1920; height = 1080; } // 1080p
            else if (selectedPresetIdx == 4) { width = 3840; height = 2160; } // 4K
            else if (selectedPresetIdx == 5) { width = 1080; height = 1080; } // Social
            else if (selectedPresetIdx == 6) { width = 1366; height = 768; } // Web
        }

        ImGui::Dummy(ImVec2(0, 2.0f));
        
        bool isCustom = (selectedPresetIdx == 0);
        if (!isCustom) {
            ImGui::BeginDisabled();
        }
        ImGui::InputInt("Width (pixels)", &width);
        ImGui::InputInt("Height (pixels)", &height);
        if (!isCustom) {
            ImGui::EndDisabled();
        }

        if (width < 1) width = 1;
        if (height < 1) height = 1;
        if (width > 16384) width = 16384;
        if (height > 16384) height = 16384;

        ImGui::InputText("Name", docName, sizeof(docName));

        const char* bgOptions[] = { "Transparent", "White", "Black" };
        ImGui::Combo("Background", &backgroundType, bgOptions, IM_ARRAYSIZE(bgOptions));

        ImGui::Dummy(ImVec2(0, 8.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 4.0f));

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
            if (backgroundType == 1) bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
            else if (backgroundType == 2) bgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Black
            
            m_CanvasView->CreateNewDocument(docName, width, height, bgColor);
            m_ShowNewDocPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_ShowNewDocPopup = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Application::Shutdown() {
    delete m_CanvasView; m_CanvasView = nullptr;
    delete m_Toolbar; m_Toolbar = nullptr;
    delete m_StatusBar; m_StatusBar = nullptr;
    delete m_PropertiesPanel; m_PropertiesPanel = nullptr;
    delete m_LayersPanel; m_LayersPanel = nullptr;
    delete m_OptionsBar; m_OptionsBar = nullptr;
    delete m_HistoryPanel; m_HistoryPanel = nullptr;
    delete m_ColorPanel; m_ColorPanel = nullptr;

    if (m_Window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_Window);
        glfwTerminate();
        m_Window = nullptr;
    }
}

void Application::RenderTitleBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float titleBarHeight = 30.0f;

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, titleBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f)); // #1f1f1f

    ImGui::Begin("##CustomTitleBar", nullptr, flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    // Draw Title Content
    std::string titleText = "Graphite Studio 2026";
    if (m_CanvasView && m_CanvasView->IsImageLoaded()) {
        int zoomPct = static_cast<int>(m_CanvasView->GetZoom() * 100.0f + 0.5f);
        titleText += " - " + m_CanvasView->GetDocumentName() + " @ " + std::to_string(zoomPct) + "%";
    }

    // Centered Title Text
    ImVec2 textSize = ImGui::CalcTextSize(titleText.c_str());
    ImGui::SetCursorPosX((viewport->Size.x - textSize.x) * 0.5f);
    ImGui::SetCursorPosY((titleBarHeight - textSize.y) * 0.5f);
    ImGui::Text("%s", titleText.c_str());

    // Far Right: Minimize, Maximize, Close Buttons
    float btnWidth = 46.0f; // Windows standard button size is roughly 46x30
    float btnHeight = 30.0f;

    // Move cursor to top right
    ImGui::SetCursorPosX(viewport->Size.x - btnWidth * 3.0f);
    ImGui::SetCursorPosY(0.0f);

    // Minimize button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    if (ImGui::Button("_##Minimize", ImVec2(btnWidth, btnHeight))) {
        glfwIconifyWindow(m_Window);
    }
    ImGui::SameLine(0, 0);

    // Maximize/Restore button
    bool isMaximized = glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
    const char* maxLabel = isMaximized ? "O##Maximize" : "[]##Maximize";
    if (ImGui::Button(maxLabel, ImVec2(btnWidth, btnHeight))) {
        if (isMaximized) {
            glfwRestoreWindow(m_Window);
        } else {
            glfwMaximizeWindow(m_Window);
        }
    }
    ImGui::SameLine(0, 0);

    // Close button (with Red Hover style)
    ImGui::PopStyleColor(2); // pop active and hover colors for Close button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.15f, 0.15f, 1.0f)); // Nice red
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.1f, 0.1f, 1.0f)); // Darker red
    if (ImGui::Button("X##Close", ImVec2(btnWidth, btnHeight))) {
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }

    ImGui::PopStyleColor(3); // Pop all colors for buttons
    ImGui::PopStyleVar(2);   // Pop button frame style vars

    ImGui::End();
}

} // namespace core
