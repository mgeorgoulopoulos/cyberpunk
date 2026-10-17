#include "01_cyberpunk.h"

#include "infra/image_io.h"
#include "effects/sunset.h"
#include "effects/grid.h"
#include "effects/knot.h"
#include "effects/robo.h"
#include "effects/rain.h"
#include "combiners/floor_reflection.h"
#include "combiners/refraction.h"
#include "combiners/bloom.h"
#include "combiners/tone.h"
#include "combiners/antialias.h"

#include <stdexcept>
#include <limits>

namespace cyber::r01_cyberpunk {
Image render(const RenderSettings& settings)
{
    const int scale = settings.aa_scale;

    Image scene(settings.width * scale, settings.height * scale, ChannelCount);
    const auto assetPath = std::filesystem::path(CP_ASSET_DIR) / "01_cyberpunk";
    const Image roboImage = loadImage(assetPath / "robo.png");
    const Image materialImage = loadImage(assetPath / "robo_material.png");

    // Scene
    sunset(scene, scale);
    grid(scene, scale);
    knot(scene);
    robo(scene, roboImage, materialImage, scale);
    floorReflection(scene, scale);

    // Rain & refract
    rain(scene, settings.seed, scale);
    refraction(scene, scale);

    // Post FX
    Image rgb(scene.width, scene.height, 3);
    copy(rgb, scene);
    const float bloomAmount = 0.8f;
    const Image glow = bloom(scene, bloomAmount, scale);
    add(rgb, glow);
    
    // AA
    return antialias(tone(rgb), scale);
}
} // namespace cyber::r01_cyberpunk



