#pragma once

#include <glad/gl.h>
#include "core/GLWrappers.h"
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

// Centralized blend mode names array for GUI combo boxes (DRY: single source of truth)
inline const char* const* GetBlendModeNames() {
    static const char* const names[] = { "Normal", "Multiply", "Screen", "Overlay", "Soft Light", "Difference" };
    return names;
}

inline constexpr int kBlendModeCount = static_cast<int>(BlendMode::Count);

class Layer {
public:
    Layer(const std::string& name, int width, int height, const unsigned char* pixels = nullptr);
    ~Layer() = default;

    // Prevent copying (since we manage OpenGL texture lifetime)
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;

    // Allow moving (Rule of Zero / Default Move Semantics)
    Layer(Layer&& other) noexcept = default;
    Layer& operator=(Layer&& other) noexcept = default;

    // Helper to upload pixel data to GPU texture
    void UploadPixels(const unsigned char* pixels);

    // Upload only a portion of the pixels (for performance during brush strokes)
    void UploadSubRect(int x, int y, int w, int h);

    // CPU pixels access
    const std::vector<unsigned char>& GetCpuPixels() const { return m_CpuPixels; }
    std::vector<unsigned char>& GetCpuPixels() { return m_CpuPixels; }

    // Properties
    std::string name;
    GLTexture texture;
    int width;
    int height;
    float opacity; // 0.0f to 1.0f
    BlendMode blendMode;
    bool visible;
    bool locked;

private:
    std::vector<unsigned char> m_CpuPixels;
};

} // namespace core
