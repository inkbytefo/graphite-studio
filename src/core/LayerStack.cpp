#include "core/LayerStack.h"
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
    
    // Read pixels back from GPU texture to duplicate it
    std::vector<unsigned char> pixels(source->width * source->height * 4);
    
    glBindTexture(GL_TEXTURE_2D, source->textureId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

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

    // In a real application, merging down involves blending the upper layer onto the lower layer.
    // We will do this using a temporary OpenGL framebuffer
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lower->textureId, 0);

    // Read lower and upper pixels from GPU
    std::vector<unsigned char> lowerPixels(lower->width * lower->height * 4);
    std::vector<unsigned char> upperPixels(upper->width * upper->height * 4);

    glBindTexture(GL_TEXTURE_2D, lower->textureId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, lowerPixels.data());

    glBindTexture(GL_TEXTURE_2D, upper->textureId);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, upperPixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Simple CPU blending for fallback merging
    // Blending math: result = upper * opacity + lower * (1 - upper_opacity)
    // In later phases, we can run this on GPU. Since it's a one-off operation, CPU fallback is extremely stable.
    float alpha = upper->opacity;
    for (int i = 0; i < lower->width * lower->height * 4; i += 4) {
        float r1 = lowerPixels[i] / 255.0f;
        float g1 = lowerPixels[i+1] / 255.0f;
        float b1 = lowerPixels[i+2] / 255.0f;
        float a1 = lowerPixels[i+3] / 255.0f;

        float r2 = upperPixels[i] / 255.0f;
        float g2 = upperPixels[i+1] / 255.0f;
        float b2 = upperPixels[i+2] / 255.0f;
        float a2 = upperPixels[i+3] / 255.0f;

        // Simple normal blending for merge down
        float outA = a2 * alpha + a1 * (1.0f - a2 * alpha);
        if (outA > 0.0f) {
            float outR = (r2 * a2 * alpha + r1 * a1 * (1.0f - a2 * alpha)) / outA;
            float outG = (g2 * a2 * alpha + g1 * a1 * (1.0f - a2 * alpha)) / outA;
            float outB = (b2 * a2 * alpha + b1 * a1 * (1.0f - a2 * alpha)) / outA;

            lowerPixels[i]   = static_cast<unsigned char>(std::clamp(outR * 255.0f, 0.0f, 255.0f));
            lowerPixels[i+1] = static_cast<unsigned char>(std::clamp(outG * 255.0f, 0.0f, 255.0f));
            lowerPixels[i+2] = static_cast<unsigned char>(std::clamp(outB * 255.0f, 0.0f, 255.0f));
            lowerPixels[i+3] = static_cast<unsigned char>(std::clamp(outA * 255.0f, 0.0f, 255.0f));
        }
    }

    // Upload merged pixels back to lower layer texture
    lower->UploadPixels(lowerPixels.data());

    // Clean up temporary FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

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
