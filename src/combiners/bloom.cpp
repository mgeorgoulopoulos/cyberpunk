#include "bloom.h"

#include "infra/blur.h"
#include <algorithm>

namespace cyber {

Image bloom(const Image& scene, float amount, int pixelScale)
{
    // RGB only
    Image bright(scene.width, scene.height, 3);
    copy(bright, scene);
    Image emission(scene.width, scene.height, 1);
    const auto c = channels(scene);
    const auto e = channels(emission);
    std::copy_n(c[Emission], emission.pixels.size(), e[0]);

    // Multiply by emissive
    combine(bright, emission, [](float color, float light) {
        return color * clamp(light, 0.0f, 5.0f);
    });

    // Blur
    Image glow = boxBlur(bright, std::max(1, scene.height / 45));
    for (float& value : glow.pixels) {
        value *= amount;
    }

    return glow;
}

} // namespace cyber

