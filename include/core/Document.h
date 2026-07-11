#pragma once

#include "core/LayerStack.h"
#include "core/HistoryManager.h"
#include "imgui.h"
#include <string>

namespace core {

class Document {
public:
    Document();
    ~Document();

    // Prevent copy
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // Load an image from disk into the document
    bool LoadFromFile(const std::string& filepath);

    // Save the composite texture (from FBO) to disk
    bool SaveCompositeToFile(const std::string& filepath, unsigned int fboId) const;

    // Create a new blank document
    bool CreateNew(const std::string& name, int width, int height, ImVec4 bgColor);

    // Clear all document state
    void Clear();

    // Accessors
    bool IsLoaded() const { return m_Loaded; }
    const std::string& GetName() const { return m_Name; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    LayerStack& GetLayerStack() { return m_LayerStack; }
    const LayerStack& GetLayerStack() const { return m_LayerStack; }

    HistoryManager& GetHistory() { return m_History; }
    const HistoryManager& GetHistory() const { return m_History; }

private:
    std::string m_Name;
    int m_Width = 0;
    int m_Height = 0;
    bool m_Loaded = false;

    LayerStack m_LayerStack;
    HistoryManager m_History;
};

} // namespace core
