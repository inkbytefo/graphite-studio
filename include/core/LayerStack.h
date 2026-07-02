#pragma once

#include "core/Layer.h"
#include <vector>
#include <memory>
#include <string>

namespace core {

class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    // Prevent copy
    LayerStack(const LayerStack&) = delete;
    LayerStack& operator=(const LayerStack&) = delete;

    // Clear stack
    void Clear();

    // Layer Operations
    Layer* AddLayer(const std::string& name, int width, int height, const unsigned char* pixels = nullptr);
    Layer* DuplicateLayer(int index);
    void DeleteLayer(int index);
    
    // Merge Operations
    void MergeDown(int index);
    void MergeVisible(int width, int height);

    // Stacking/Reordering
    void MoveLayer(int fromIndex, int toIndex);

    // Getters / Setters
    int GetCount() const { return static_cast<int>(m_Layers.size()); }
    Layer* GetLayer(int index) const;
    
    int GetSelectedIndex() const { return m_SelectedIndex; }
    void SetSelectedIndex(int index);
    Layer* GetSelectedLayer() const;

private:
    std::vector<std::unique_ptr<Layer>> m_Layers;
    int m_SelectedIndex;
};

} // namespace core
