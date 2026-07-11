#include "core/Layer.h"
#include <utility>
#include <iostream>

namespace core {

Layer::Layer(const std::string& name, int width, int height, const unsigned char* pixels)
    : name(name), textureId(0), width(width), height(height),
      opacity(1.0f), blendMode(BlendMode::Normal), visible(true), locked(false) {
    
    std::cout << "[Layer] Creating layer '" << name << "' (" << width << "x" << height << ")" << std::endl;

    // Allocate CPU storage
    m_CpuPixels.resize(width * height * 4, 0);
    if (pixels) {
        std::copy(pixels, pixels + (width * height * 4), m_CpuPixels.begin());
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[Layer] OpenGL error before texture creation: " << err << std::endl;
    }

    // Generate OpenGL texture for the layer using modern DSA (Direct State Access)
    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
    
    // Set typical Photoshop texture parameters (Nearest neighbor for sharp pixels) using DSA
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Set unpack alignment to 1 to prevent driver crashes on odd-width images
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Allocate immutable GPU storage and upload using DSA
    glTextureStorage2D(textureId, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());

    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[Layer] glTextureStorage2D/SubImage failed for layer '" << name << "' with error: " << err << std::endl;
    } else {
        std::cout << "[Layer] GPU texture " << textureId << " created successfully via DSA." << std::endl;
    }
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
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void Layer::UploadSubRect(int x, int y, int w, int h) {
    if (textureId == 0) return;
    
    // Clamp coordinates to layer boundaries
    x = std::max(0, std::min(x, width - 1));
    y = std::max(0, std::min(y, height - 1));
    w = std::max(1, std::min(w, width - x));
    h = std::max(1, std::min(h, height - y));

    // Tell OpenGL how to read a sub-rectangle out of our contiguous CPU pixels vector
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

    glTextureSubImage2D(textureId, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, m_CpuPixels.data());

    // Restore default unpack state
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
}

} // namespace core
