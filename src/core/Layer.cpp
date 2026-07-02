#include "core/Layer.h"
#include <utility>

namespace core {

Layer::Layer(const std::string& name, int width, int height, const unsigned char* pixels)
    : name(name), textureId(0), width(width), height(height),
      opacity(1.0f), blendMode(BlendMode::Normal), visible(true), locked(false) {
    
    // Generate OpenGL texture for the layer
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set typical Photoshop texture parameters (Nearest neighbor for sharp pixels)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate GPU storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (pixels) {
        UploadPixels(pixels);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Layer::~Layer() {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
}

Layer::Layer(Layer&& other) noexcept
    : name(std::move(other.name)),
      textureId(other.textureId),
      width(other.width),
      height(other.height),
      opacity(other.opacity),
      blendMode(other.blendMode),
      visible(other.visible),
      locked(other.locked) {
    other.textureId = 0;
}

Layer& Layer::operator=(Layer&& other) noexcept {
    if (this != &other) {
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
        }
        name = std::move(other.name);
        textureId = other.textureId;
        width = other.width;
        height = other.height;
        opacity = other.opacity;
        blendMode = other.blendMode;
        visible = other.visible;
        locked = other.locked;
        other.textureId = 0;
    }
    return *this;
}

void Layer::UploadPixels(const unsigned char* pixels) {
    if (textureId == 0 || !pixels) return;
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace core
