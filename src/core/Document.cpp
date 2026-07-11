#include "core/Document.h"
#include "core/Image.h"
#include <glad/gl.h>
#include <algorithm>
#include <iostream>
#include <vector>

namespace core {

Document::Document() : m_Loaded(false), m_Width(0), m_Height(0) {}

Document::~Document() {
    Clear();
}

void Document::Clear() {
    m_LayerStack.Clear();
    m_History.Clear();
    m_Name.clear();
    m_Width = 0;
    m_Height = 0;
    m_Loaded = false;
}

bool Document::LoadFromFile(const std::string& filepath) {
    Clear();

    core::Image tempImage;
    if (!tempImage.LoadFromFile(filepath)) {
        return false;
    }

    m_Width = tempImage.GetWidth();
    m_Height = tempImage.GetHeight();
    
    // Extrct file name from path
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        m_Name = filepath.substr(lastSlash + 1);
    } else {
        m_Name = filepath;
    }

    m_LayerStack.AddLayer("Background", m_Width, m_Height, tempImage.GetPixels());
    m_Loaded = true;

    m_History.RecordState(m_LayerStack, "Open Image");
    return true;
}

bool Document::SaveCompositeToFile(const std::string& filepath, unsigned int fboId) const {
    if (!m_Loaded || fboId == 0) return false;

    std::vector<unsigned char> pixels(m_Width * m_Height * 4);

    GLint lastFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_FRAMEBUFFER, lastFbo);

    // Flip the image vertically for file output (OpenGL coordinate correction)
    std::vector<unsigned char> flippedPixels(m_Width * m_Height * 4);
    int rowSize = m_Width * 4;
    for (int y = 0; y < m_Height; ++y) {
        std::copy(
            pixels.data() + y * rowSize,
            pixels.data() + (y + 1) * rowSize,
            flippedPixels.data() + (m_Height - 1 - y) * rowSize
        );
    }

    core::Image tempImg(m_Width, m_Height, 4);
    std::copy(flippedPixels.begin(), flippedPixels.end(), tempImg.GetPixels());
    return tempImg.SaveToFile(filepath);
}

bool Document::CreateNew(const std::string& name, int width, int height, ImVec4 bgColor) {
    Clear();

    m_Name = name;
    m_Width = width;
    m_Height = height;

    std::vector<unsigned char> pixels(width * height * 4);
    unsigned char r = static_cast<unsigned char>(std::clamp(bgColor.x * 255.0f, 0.0f, 255.0f));
    unsigned char g = static_cast<unsigned char>(std::clamp(bgColor.y * 255.0f, 0.0f, 255.0f));
    unsigned char b = static_cast<unsigned char>(std::clamp(bgColor.z * 255.0f, 0.0f, 255.0f));
    unsigned char a = static_cast<unsigned char>(std::clamp(bgColor.w * 255.0f, 0.0f, 255.0f));
    
    for (int i = 0; i < width * height * 4; i += 4) {
        pixels[i]   = r;
        pixels[i+1] = g;
        pixels[i+2] = b;
        pixels[i+3] = a;
    }
    
    m_LayerStack.AddLayer(name, width, height, pixels.data());
    m_Loaded = true;

    m_History.RecordState(m_LayerStack, "New Document");
    return true;
}

} // namespace core
