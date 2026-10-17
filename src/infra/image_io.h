#pragma once
#include "infra/types.h"
#include <filesystem>

namespace cyber {
// Load png as RGBA
Image loadImage(const std::filesystem::path& path);

// Write png as RGB (when 3 channels provided) or RGBA (4 channels provided).
void saveImage(const Image& image, const std::filesystem::path& path);
}
