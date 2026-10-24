#include "01_cyberpunk.h"

#include "infra/image_io.h"
#include "infra/random_uniform.h"
#include "effects/sunset.h"
#include "effects/grid.h"
#include "effects/knot.h"
#include "effects/robo.h"
#include "effects/rain.h"
#include "combiners/color_grader.h"
#include "combiners/floor_reflection.h"
#include "combiners/refraction.h"
#include "combiners/bloom.h"
#include "combiners/tone.h"
#include "combiners/antialias.h"

#include <stdexcept>
#include <limits>

namespace cyber::r01_cyberpunk {
Image render(const RenderSettings &settings) {
    RandomUniform random(settings.seed);

    const int scale = settings.aaScale;

    Image scene(settings.width * scale, settings.height * scale, ChannelCount);
    const auto assetPath = std::filesystem::path("./assets") / "01_cyberpunk";
    const Image roboImage = loadImage(assetPath / "robo.png");
    const Image materialImage = loadImage(assetPath / "robo_material.png");
    const Image grader = loadImage(assetPath / "color_grader.png");

    // Scene
    sunset(scene, scale, random.nextSeed());
    grid(scene, scale, random.nextSeed());
    knot(scene);
    robo(scene, roboImage, materialImage, scale);
    floorReflection(scene, random.nextSeed());

    // Rain & refract
    rain(scene, scale, random.nextSeed());
    refraction(scene, scale);

    // Post FX (HDR)
    Image rgb(scene.width, scene.height, 3);
    copy(rgb, scene);
    const float bloomAmount = 0.8f;
    const Image glow = bloom(scene, bloomAmount, scale);
    add(rgb, glow);

    // Post-post
    rgb = tone(rgb);
    rgb = antialias(rgb, scale);
    rgb = colorGrade(rgb, grader);

    return rgb;
}
} // namespace cyber::r01_cyberpunk
