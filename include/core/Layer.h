#pragma once

#include <glad/gl.h>
#include <string>
#include <vector>

namespace core {

enum class BlendMode {
    Normal = 0,
    Multiply,
    Screen,
    Overlay,
    SoftLight,
    Difference,
    Count
};

// Helper to get blend mode names
inline const char* GetBlendModeName(BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal:     return "Normal";
        case BlendMode::Multiply:   return "Multiply";
        case BlendMode::Screen:     return "Screen";
        case BlendMode::Overlay:    return "Overlay";
        case BlendMode::SoftLight:  return "Soft Light";
        case BlendMode::Difference: return "Difference";
        default:                    return "Unknown";
    }
}

class Layer {
public:
    Layer(const std::string& name, int width, int height, const unsigned char* pixels = nullptr);
    ~Layer();

    // Prevent copying (since we manage OpenGL texture lifetime)
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;

    // Allow moving
    Layer(Layer&& other) noexcept;
    Layer& operator=(Layer&& other) noexcept;

    // Helper to upload pixel data to GPU texture
    void UploadPixels(const unsigned char* pixels);

    // Properties
    std::string name;
    GLuint textureId;
    int width;
    int height;
    float opacity; // 0.0f to 1.0f
    BlendMode blendMode;
    bool visible;
    bool locked;
};

} // namespace core
