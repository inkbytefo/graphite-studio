#include "core/LayerStack.h"
#include "core/BlendMath.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace core {

LayerStack::LayerStack() : m_SelectedIndex(-1) {}

LayerStack::~LayerStack() {
    Clear();
}

void LayerStack::Clear() {
    m_Layers.clear();
    m_SelectedIndex = -1;
}

Layer* LayerStack::AddLayer(const std::string& name, int width, int height, const unsigned char* pixels) {
    auto layer = std::make_unique<Layer>(name, width, height, pixels);
    Layer* rawPtr = layer.get();
    
    // Add to the top of the stack (which is end of the vector)
    m_Layers.push_back(std::move(layer));
    m_SelectedIndex = static_cast<int>(m_Layers.size()) - 1;
    
    return rawPtr;
}

Layer* LayerStack::DuplicateLayer(int index) {
    if (index < 0 || index >= GetCount()) return nullptr;
    
    Layer* source = m_Layers[index].get();
    
    // Read pixels from CPU cache to duplicate it (avoiding slow GPU read-back)
    const auto& pixels = source->GetCpuPixels();

    std::string newName = source->name + " Copy";
    auto dup = std::make_unique<Layer>(newName, source->width, source->height, pixels.data());
    dup->opacity = source->opacity;
    dup->blendMode = source->blendMode;
    dup->visible = source->visible;
    dup->locked = source->locked;
    
    Layer* rawPtr = dup.get();
    
    // Insert immediately above the source layer
    m_Layers.insert(m_Layers.begin() + index + 1, std::move(dup));
    m_SelectedIndex = index + 1;
    
    return rawPtr;
}

void LayerStack::DeleteLayer(int index) {
    if (index < 0 || index >= GetCount()) return;
    
    // Photoshop behavior: Cannot delete the last remaining layer
    if (m_Layers.size() <= 1) return;

    m_Layers.erase(m_Layers.begin() + index);

    // Adjust selected index
    if (m_SelectedIndex >= static_cast<int>(m_Layers.size())) {
        m_SelectedIndex = static_cast<int>(m_Layers.size()) - 1;
    }
}

void LayerStack::MoveLayer(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= GetCount() || toIndex < 0 || toIndex >= GetCount()) return;
    if (fromIndex == toIndex) return;

    // Save selected layer pointer to restore selection after reordering
    Layer* selectedLayer = GetSelectedLayer();

    auto targetIter = m_Layers.begin() + fromIndex;
    auto moveLayer = std::move(*targetIter);
    m_Layers.erase(targetIter);
    
    m_Layers.insert(m_Layers.begin() + toIndex, std::move(moveLayer));

    // Restore selected index
    for (size_t i = 0; i < m_Layers.size(); ++i) {
        if (m_Layers[i].get() == selectedLayer) {
            m_SelectedIndex = static_cast<int>(i);
            break;
        }
    }
}

void LayerStack::MergeDown(int index) {
    // Cannot merge down the bottom-most layer
    if (index <= 0 || index >= GetCount()) return;

    Layer* upper = m_Layers[index].get();
    Layer* lower = m_Layers[index - 1].get();

    // Read lower and upper pixels from CPU cache (avoiding slow GPU read-back)
    const auto& upperPixels = upper->GetCpuPixels();
    auto& lowerPixels = lower->GetCpuPixels();

    float alpha = upper->opacity;
    for (size_t i = 0; i < lowerPixels.size(); i += 4) {
        float r1 = ByteToFloat(lowerPixels[i]);
        float g1 = ByteToFloat(lowerPixels[i+1]);
        float b1 = ByteToFloat(lowerPixels[i+2]);
        float a1 = ByteToFloat(lowerPixels[i+3]);

        float r2 = ByteToFloat(upperPixels[i]);
        float g2 = ByteToFloat(upperPixels[i+1]);
        float b2 = ByteToFloat(upperPixels[i+2]);
        float a2 = ByteToFloat(upperPixels[i+3] * alpha);

        AlphaCompositeOver(r2, g2, b2, a2, r1, g1, b1, a1);

        lowerPixels[i]   = FloatToByte(r1);
        lowerPixels[i+1] = FloatToByte(g1);
        lowerPixels[i+2] = FloatToByte(b1);
        lowerPixels[i+3] = FloatToByte(a1);
    }

    // Upload merged pixels back to lower layer texture
    lower->UploadPixels(lowerPixels.data());

    // Remove the upper layer
    DeleteLayer(index);
    m_SelectedIndex = index - 1;
}

void LayerStack::MergeVisible(int width, int height) {
    // Merges all visible layers into a single background layer
    if (GetCount() <= 1) return;

    // We will simulate merging all visible layers. In later steps, Compositor will handle it.
    // For now, we will notify that Compositor's result should be copied.
    // Actually, we can implement it as a combination of merge downs.
    for (int i = GetCount() - 1; i > 0; --i) {
        if (m_Layers[i]->visible) {
            MergeDown(i);
        }
    }
}

Layer* LayerStack::GetLayer(int index) const {
    if (index < 0 || index >= GetCount()) return nullptr;
    return m_Layers[index].get();
}

void LayerStack::SetSelectedIndex(int index) {
    if (index >= -1 && index < GetCount()) {
        m_SelectedIndex = index;
    }
}

Layer* LayerStack::GetSelectedLayer() const {
    return GetLayer(m_SelectedIndex);
}

} // namespace core
