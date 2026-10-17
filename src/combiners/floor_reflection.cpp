#include "floor_reflection.h"

#include "infra/noise.h"
#include "infra/sample.h"

namespace cyber {

namespace {
struct Pixel {
    Tensor<3> color;
    float emission{};
};
Pixel shader(const Image &environment, float x, float y, float reflectivity, float ripple, int pixelScale) {
    Pixel pixel;

    ripple -= 0.5f;

    float dx = ripple * 35 * pixelScale + std::sin((y * 0.35f + x * 0.015f) / pixelScale) * 2.5f * pixelScale;
    float dy = ripple * 8 * pixelScale + std::sin((x * 0.11f + y * 0.20f) / pixelScale) * 1.3f * pixelScale;

    float rx = x + dx;
    float ry = environment.height * 1.19f - y + dy;
    float roboY = environment.height * 1.80f - y + dy;

    float roboMask = 0.0f;
    if (y > environment.height * 0.90f) {
        roboMask = sample(environment, Character, rx, roboY);
    }
        
    for (int c = 0; c < 3; ++c) {
        pixel.color[c] = lerp(
            sample(environment, c, rx, ry),
            sample(environment, c, rx, roboY),
            roboMask);
        pixel.color[c] =
            sample(environment, c, x, y) * (1 - reflectivity * 0.48f)
            + pixel.color[c] * reflectivity * 0.72f;
    }

    pixel.emission =
        sample(environment, Emission, x, y) +
        sample(environment, Emission, rx, ry) * reflectivity * 0.32f;

    return pixel;
}

} // namespace

void floorReflection(Image& scene, int pixelScale, RandomSeed seed)
{
    Image source = scene;
    Image ripples = noise(scene.width, scene.height, seed, 0.2f);

    auto c = channels(scene);
    auto cc = channels(source);
    auto r = channels(ripples);

    int offset = 0;
    for (int j = 0; j < scene.height; j++) {
        for (int i = 0; i < scene.width; i++) {

            const float reflectivity = cc[Reflectivity][offset];

            if (reflectivity > 0) {
                const float ripple = r[0][offset];

                const Pixel p = shader(source, i, j, reflectivity, ripple, pixelScale);

                c[R][offset] = p.color[0];
                c[G][offset] = p.color[1];
                c[B][offset] = p.color[2];
                c[Emission][offset] = p.emission;
            }


            offset++;
        }
    }
}

} // cyber