#pragma once
#include "core/GLWrappers.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace gui {

class IconHelper {
public:
    static std::shared_ptr<core::GLTexture> GetIcon(const std::string& name, int w = 24, int h = 24);
    static void Shutdown();

private:
    static std::unordered_map<std::string, std::shared_ptr<core::GLTexture>> s_IconCache;
};

} // namespace gui
