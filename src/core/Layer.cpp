#include "core/Layer.h"
#include <utility>

namespace core {

Layer::Layer(const std::string& name, int width, int height, const unsigned char* pixels)
    : name(name), textureId(0), width(width), height(height),
      opacity(1.0f), blendMode(BlendMode::Normal), visible(true), locked(false) {
    
    // Allocate CPU storage
    m_CpuPixels.resize(width * height * 4, 0);
    if (pixels) {
        std::copy(pixels, pixels + (width * height * 4), m_CpuPixels.begin());
    }

    // Generate OpenGL texture for the layer
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set typical Photoshop texture parameters (Nearest neighbor for sharp pixels)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate GPU storage and upload
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());

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
      locked(other.locked),
      m_CpuPixels(std::move(other.m_CpuPixels)) {
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
        m_CpuPixels = std::move(other.m_CpuPixels);
        other.textureId = 0;
    }
    return *this;
}

void Layer::UploadPixels(const unsigned char* pixels) {
    if (textureId == 0 || !pixels) return;
    m_CpuPixels.assign(pixels, pixels + (width * height * 4));
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Layer::UploadSubRect(int x, int y, int w, int h) {
    if (textureId == 0) return;
    
    // Clamp coordinates to layer boundaries
    x = std::max(0, std::min(x, width - 1));
    y = std::max(0, std::min(y, height - 1));
    w = std::max(1, std::min(w, width - x));
    h = std::max(1, std::min(h, height - y));

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Tell OpenGL how to read a sub-rectangle out of our contiguous CPU pixels vector
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());

    // Restore default unpack state
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace core
