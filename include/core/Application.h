#pragma once

struct GLFWwindow;

namespace gui {
class CanvasView;
class Toolbar;
class StatusBar;
class PropertiesPanel;
class LayersPanel;
class OptionsBar;
class HistoryPanel;
class ColorPanel;
}

namespace core {

class Application {
public:
    Application();
    ~Application();

    // Initialize window, OpenGL, and ImGui
    bool Init();

    // Run the main event loop
    void Run();

    // Shutdown and clean up resources
    void Shutdown();

private:
    void HandleGlobalShortcuts();
    void RenderMenuBar(bool& reset_layout);
    void RenderNewDocumentPopup();
    void SetupDockSpace(bool& reset_layout);
    void RenderTitleBar();

    GLFWwindow* m_Window = nullptr;

    // GUI panels and views
    gui::CanvasView* m_CanvasView = nullptr;
    gui::Toolbar* m_Toolbar = nullptr;
    gui::StatusBar* m_StatusBar = nullptr;
    gui::PropertiesPanel* m_PropertiesPanel = nullptr;
    gui::LayersPanel* m_LayersPanel = nullptr;
    gui::OptionsBar* m_OptionsBar = nullptr;
    gui::HistoryPanel* m_HistoryPanel = nullptr;
    gui::ColorPanel* m_ColorPanel = nullptr;

    // UI state flags
    bool m_ShowDemoWindow = false;
    bool m_ShowNewDocPopup = false;
    bool m_ResetLayout = false;
    float m_ClearColor[4] = {0.118f, 0.118f, 0.118f, 1.00f};
};

} // namespace core
