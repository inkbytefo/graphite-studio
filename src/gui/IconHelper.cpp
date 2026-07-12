#include "gui/IconHelper.h"
#include <iostream>
#include <vector>

#define NANOSVG_IMPLEMENTATION
#include "../../thirdparty/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "../../thirdparty/nanosvgrast.h"

#ifdef _WIN32
#include <windows.h>
static std::string GetExecutableDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string::size_type pos = std::string(path).find_last_of("\\/");
    if (pos != std::string::npos) {
        return std::string(path).substr(0, pos);
    }
    return "";
}
#endif

namespace gui {

std::unordered_map<std::string, std::shared_ptr<core::GLTexture>> IconHelper::s_IconCache;

std::shared_ptr<core::GLTexture> IconHelper::GetIcon(const std::string& name, int w, int h) {
    auto it = s_IconCache.find(name);
    if (it != s_IconCache.end()) {
        return it->second;
    }

    std::string resolvedPath = "";
    NSVGimage* svgImage = nullptr;

    std::vector<std::string> candidates = {
        "resources/icons/" + name
    };

#ifdef _WIN32
    std::string exeDir = GetExecutableDirectory();
    if (!exeDir.empty()) {
        candidates.push_back(exeDir + "/resources/icons/" + name);
        candidates.push_back(exeDir + "/../../resources/icons/" + name);
        candidates.push_back(exeDir + "/../../../resources/icons/" + name);
    }
#endif

    for (const auto& candidate : candidates) {
        svgImage = nsvgParseFromFile(candidate.c_str(), "px", 96.0f);
        if (svgImage) {
            resolvedPath = candidate;
            break;
        }
    }

    if (!svgImage) {
        std::cerr << "[IconHelper] Failed to parse SVG file: " << name << " (Tried multiple path fallbacks)" << std::endl;
        return nullptr;
    }

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(svgImage);
        return nullptr;
    }

    std::vector<unsigned char> imgData(w * h * 4);
    // Rasterize SVG to RGBA pixels
    nsvgRasterize(rast, svgImage, 0, 0, 1.0f, imgData.data(), w, h, w * 4);

    auto texture = std::make_shared<core::GLTexture>();
    texture->Create(GL_TEXTURE_2D);
    texture->AllocateStorage2D(1, GL_RGBA8, w, h);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    texture->UploadSubImage2D(0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, imgData.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    texture->SetParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture->SetParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(svgImage);

    s_IconCache[name] = texture;
    return texture;
}

void IconHelper::Shutdown() {
    s_IconCache.clear();
}

} // namespace gui
