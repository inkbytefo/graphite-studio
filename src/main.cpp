#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#endif

#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gui/Theme.h"
#include "gui/Workspace.h"
#include "gui/CanvasView.h"
#include "gui/Toolbar.h"
#include "gui/StatusBar.h"
#include "gui/PropertiesPanel.h"
#include "gui/LayersPanel.h"
#include "core/Layer.h"
#include "core/LayerStack.h"

// Windows-specific implementation for native file open dialog
#ifdef _WIN32
std::string OpenFileDialog() {
    char szFile[260] = { 0 };
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Görsel Dosyaları (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0Tüm Dosyalar (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}
#else
std::string OpenFileDialog() {
    return ""; // Fallback for other platforms
}
#endif

// GLFW error callback
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(1600, 900, "Graphite Studio 2026", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style and fonts
    gui::ApplyPhotoshopTheme();
    gui::InitFonts(io);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize UI Panels and Views
    gui::CanvasView canvasView;
    canvasView.Init();

    gui::Toolbar toolbar;
    gui::StatusBar statusBar;
    gui::PropertiesPanel propertiesPanel;
    gui::LayersPanel layersPanel;

    // State
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.118f, 0.118f, 0.118f, 1.00f); // Match canvas background

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        bool reset_layout = false;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Global Keyboard Shortcuts (must be AFTER NewFrame for valid input state)
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
            std::string filepath = OpenFileDialog();
            if (!filepath.empty()) {
                canvasView.LoadImageFromFile(filepath);
            }
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_0) || ImGui::IsKeyPressed(ImGuiKey_Keypad0))) {
            canvasView.ResetView();
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_1) || ImGui::IsKeyPressed(ImGuiKey_Keypad1))) {
            canvasView.ZoomToActual();
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) || ImGui::IsKeyPressed(ImGuiKey_Equal))) {
            canvasView.ZoomIn();
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) || ImGui::IsKeyPressed(ImGuiKey_Minus))) {
            canvasView.ZoomOut();
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Apostrophe)) {
            canvasView.TogglePixelGrid();
        }
        
        // ─── Layer stack shortcuts (Photoshop-accurate) ───
        if (canvasView.IsImageLoaded()) {
            core::LayerStack& stack = canvasView.GetLayerStack();
            int selIdx = stack.GetSelectedIndex();
            int count = stack.GetCount();
            
            // Ctrl+Shift+N: New Layer
            if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_N)) {
                char name[64];
                snprintf(name, sizeof(name), "Layer %d", count + 1);
                stack.AddLayer(name, canvasView.GetImageWidth(), canvasView.GetImageHeight(), nullptr);
            }
            
            // Ctrl+J: Duplicate Layer
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_J)) {
                if (selIdx != -1) {
                    stack.DuplicateLayer(selIdx);
                }
            }
            
            // Delete: Delete selected layer (if not typing in text inputs)
            if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::GetIO().WantTextInput) {
                if (selIdx != -1) {
                    stack.DeleteLayer(selIdx);
                }
            }
            
            // Ctrl+E: Merge Down
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E)) {
                if (selIdx > 0) {
                    stack.MergeDown(selIdx);
                }
            }
            
            // Alt+[ : Select next layer down
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
                if (selIdx > 0) {
                    stack.SetSelectedIndex(selIdx - 1);
                }
            }
            
            // Alt+] : Select next layer up
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
                if (selIdx < count - 1) {
                    stack.SetSelectedIndex(selIdx + 1);
                }
            }
            
            // Ctrl+[ : Move layer down
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_LeftBracket)) {
                if (selIdx > 0) {
                    stack.MoveLayer(selIdx, selIdx - 1);
                }
            }
            
            // Ctrl+] : Move layer up
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_RightBracket)) {
                if (selIdx < count - 1) {
                    stack.MoveLayer(selIdx, selIdx + 1);
                }
            }
        }

        // 1. Create the Main DockSpace filling the whole viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // We leave room at the bottom for the status bar
        float statusBarHeight = 24.0f;
        ImVec2 workPos = viewport->WorkPos;
        ImVec2 workSize = viewport->WorkSize;
        workSize.y -= statusBarHeight;

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
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        }

        // Setup programmatic layout if it hasn't been set up yet
        gui::SetupDefaultLayout(dockspace_id, reset_layout);

        // 2. Render Main Menu Bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    std::string filepath = OpenFileDialog();
                    if (!filepath.empty()) {
                        canvasView.LoadImageFromFile(filepath);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
                ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Image")) {
                ImGui::MenuItem("Image Size...", nullptr, false, false);
                ImGui::MenuItem("Canvas Size...", nullptr, false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Layer")) {
                bool hasDoc = canvasView.IsImageLoaded();
                core::LayerStack& stack = canvasView.GetLayerStack();
                int selIdx = stack.GetSelectedIndex();
                
                if (ImGui::MenuItem("New Layer", "Ctrl+Shift+N", false, hasDoc)) {
                    char name[64];
                    snprintf(name, sizeof(name), "Layer %d", stack.GetCount() + 1);
                    stack.AddLayer(name, canvasView.GetImageWidth(), canvasView.GetImageHeight(), nullptr);
                }
                if (ImGui::MenuItem("Duplicate Layer", "Ctrl+J", false, hasDoc && selIdx != -1)) {
                    stack.DuplicateLayer(selIdx);
                }
                if (ImGui::MenuItem("Delete Layer", "Delete", false, hasDoc && selIdx != -1 && stack.GetCount() > 1)) {
                    stack.DeleteLayer(selIdx);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Merge Down", "Ctrl+E", false, hasDoc && selIdx > 0)) {
                    stack.MergeDown(selIdx);
                }
                if (ImGui::MenuItem("Merge Visible", "Ctrl+Shift+E", false, hasDoc && stack.GetCount() > 1)) {
                    stack.MergeVisible(canvasView.GetImageWidth(), canvasView.GetImageHeight());
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
                    canvasView.ZoomIn();
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    canvasView.ZoomOut();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Fit on Screen", "Ctrl+0")) {
                    canvasView.ResetView();
                }
                if (ImGui::MenuItem("Actual Pixels (100%)", "Ctrl+1")) {
                    canvasView.ZoomToActual();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Show")) {
                    bool pixelGrid = canvasView.IsPixelGridVisible();
                    if (ImGui::MenuItem("Pixel Grid", "Ctrl+'", &pixelGrid)) {
                        if (pixelGrid != canvasView.IsPixelGridVisible()) {
                            canvasView.TogglePixelGrid();
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                ImGui::MenuItem("ImGui Demo Window", nullptr, &show_demo_window);
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

        // End DockSpace window
        ImGui::End();

        // 3. Render all UI Panels
        toolbar.Render();

        // Sync active tool between Toolbar and CanvasView (bidirectional)
        canvasView.SetActiveTool(toolbar.GetActiveTool());

        propertiesPanel.Render();
        layersPanel.Render(canvasView);
        canvasView.Render();

        // Update StatusBar values and render it
        statusBar.SetHasDocument(canvasView.IsImageLoaded());
        if (canvasView.IsImageLoaded()) {
            statusBar.SetZoom(canvasView.GetZoom());
            statusBar.SetDocumentSize(canvasView.GetImageWidth(), canvasView.GetImageHeight());
            float mx, my;
            canvasView.GetMouseImageCoords(mx, my);
            statusBar.SetMousePos(mx, my);
        }
        statusBar.Render();

        // Show Demo Window if checked
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows (if multi-viewport enabled)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
