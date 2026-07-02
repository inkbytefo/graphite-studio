#include "core/Image.h"
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

namespace core {

Image::Image()
    : m_Width(0), m_Height(0), m_Channels(0) {}

Image::Image(int width, int height, int channels) {
    CreateEmpty(width, height, channels);
}

Image::~Image() {
    Clear();
}

bool Image::LoadFromFile(const std::string& filepath) {
    Clear();

    // Force 4 channels (RGBA) for OpenGL compatibility
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image: " << filepath << " Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    m_Width = width;
    m_Height = height;
    m_Channels = 4; // Since we forced 4 channels
    m_Pixels.assign(data, data + (m_Width * m_Height * m_Channels));

    stbi_image_free(data);
    return true;
}

bool Image::SaveToFile(const std::string& filepath) const {
    if (!IsLoaded()) {
        std::cerr << "Cannot save empty image!" << std::endl;
        return false;
    }

    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        std::cerr << "Invalid output file path (no extension)!" << std::endl;
        return false;
    }

    std::string ext = filepath.substr(dotPos + 1);
    for (auto& c : ext) c = std::tolower(c);

    int result = 0;
    if (ext == "png") {
        result = stbi_write_png(filepath.c_str(), m_Width, m_Height, m_Channels, m_Pixels.data(), m_Width * m_Channels);
    } else if (ext == "jpg" || ext == "jpeg") {
        result = stbi_write_jpg(filepath.c_str(), m_Width, m_Height, m_Channels, m_Pixels.data(), 90);
    } else if (ext == "bmp") {
        result = stbi_write_bmp(filepath.c_str(), m_Width, m_Height, m_Channels, m_Pixels.data());
    } else if (ext == "tga") {
        result = stbi_write_tga(filepath.c_str(), m_Width, m_Height, m_Channels, m_Pixels.data());
    } else {
        std::cerr << "Unsupported file extension: " << ext << ". Saving as PNG." << std::endl;
        std::string pngPath = filepath.substr(0, dotPos) + ".png";
        result = stbi_write_png(pngPath.c_str(), m_Width, m_Height, m_Channels, m_Pixels.data(), m_Width * m_Channels);
    }

    return result != 0;
}

void Image::CreateEmpty(int width, int height, int channels, unsigned char fillValue) {
    Clear();
    m_Width = width;
    m_Height = height;
    m_Channels = channels;
    m_Pixels.resize(m_Width * m_Height * m_Channels, fillValue);
}

void Image::Clear() {
    m_Width = 0;
    m_Height = 0;
    m_Channels = 0;
    m_Pixels.clear();
}

} // namespace core
