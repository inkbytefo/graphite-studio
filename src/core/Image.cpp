#include "core/Image.h"
#include "platform/FileDialogs.h"
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
    unsigned char* data = nullptr;

#ifdef _WIN32
    // Convert UTF-8 path to UTF-16 wide path for Windows
    std::wstring wpath = platform::UTF8ToWString(filepath);

    // Open file using _wfopen to support non-ASCII (Turkish, etc.) characters in path
    FILE* f = _wfopen(wpath.c_str(), L"rb");
    if (!f) {
        std::cerr << "Failed to open file (Unicode): " << filepath << std::endl;
        return false;
    }
    data = stbi_load_from_file(f, &width, &height, &channels, 4);
    fclose(f);
#else
    data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
#endif

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

static void stbi_write_func_file(void* context, void* data, int size) {
    fwrite(data, 1, size, static_cast<FILE*>(context));
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

    FILE* f = nullptr;
#ifdef _WIN32
    std::wstring wpath = platform::UTF8ToWString(filepath);
    f = _wfopen(wpath.c_str(), L"wb");
#else
    f = fopen(filepath.c_str(), "wb");
#endif

    if (!f) {
        std::cerr << "Failed to open output file: " << filepath << std::endl;
        return false;
    }

    int result = 0;
    if (ext == "png") {
        result = stbi_write_png_to_func(stbi_write_func_file, f, m_Width, m_Height, m_Channels, m_Pixels.data(), m_Width * m_Channels);
    } else if (ext == "jpg" || ext == "jpeg") {
        result = stbi_write_jpg_to_func(stbi_write_func_file, f, m_Width, m_Height, m_Channels, m_Pixels.data(), 90);
    } else if (ext == "bmp") {
        result = stbi_write_bmp_to_func(stbi_write_func_file, f, m_Width, m_Height, m_Channels, m_Pixels.data());
    } else if (ext == "tga") {
        result = stbi_write_tga_to_func(stbi_write_func_file, f, m_Width, m_Height, m_Channels, m_Pixels.data());
    } else {
        std::cerr << "Unsupported file extension: " << ext << ". Saving as PNG." << std::endl;
        result = stbi_write_png_to_func(stbi_write_func_file, f, m_Width, m_Height, m_Channels, m_Pixels.data(), m_Width * m_Channels);
    }

    fclose(f);
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
