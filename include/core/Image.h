#pragma once

#include <string>
#include <vector>

namespace core {

class Image {
public:
    Image();
    Image(int width, int height, int channels = 4);
    ~Image();

    // Load from disk (PNG, JPG, TGA, BMP, PSD)
    bool LoadFromFile(const std::string& filepath);
    
    // Save to disk (detects format from extension: png, jpg, bmp, tga)
    bool SaveToFile(const std::string& filepath) const;

    // Create a blank image
    void CreateEmpty(int width, int height, int channels = 4, unsigned char fillValue = 255);

    // Getters
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    int GetChannels() const { return m_Channels; }
    const unsigned char* GetPixels() const { return m_Pixels.data(); }
    unsigned char* GetPixels() { return m_Pixels.data(); }

    bool IsLoaded() const { return !m_Pixels.empty(); }

    // Clear memory
    void Clear();

private:
    int m_Width;
    int m_Height;
    int m_Channels;
    std::vector<unsigned char> m_Pixels;
};

} // namespace core
