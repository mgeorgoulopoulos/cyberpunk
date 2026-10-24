#include "floor_reflection.h"

#include "infra/noise.h"
#include "infra/sample.h"

namespace cyber {

namespace {
struct Pixel {
    Tensor<3> color;
    float emission{};
};
Pixel shader(const Image &source, const Image &environment, float x, float y, float reflectivity, float ripple) {
    Pixel pixel;

    ripple -= 0.5f;

    const float waveSize = environment.width / 1280.0f;
    const float phaseX = (y * 0.14f + x * 0.004f) / waveSize;
    const float phaseY = (x * 0.025f + y * 0.09f) / waveSize;

    // irregular waves
    const float wavyness = clamp01(reflectivity);
    const float waveX = lerp(std::sin(phaseX * 1.6f), std::sin(phaseX * 0.6f), wavyness);
    const float waveY = lerp(std::sin(phaseY * 1.6f), std::sin(phaseY * 0.6f), wavyness);

    // fade with distance
    const float z = sample(source, Depth, x, y);
    const float depthFade = 4.2f / std::max(4.2f, z);
    float dx = (ripple * 70 + waveX * 9) * waveSize * depthFade;
    float dy = (ripple * 12 + waveY * 3) * waveSize * depthFade;

    float rx = x + dx;
    float ry = environment.height * 1.19f - y + dy;

    // softer reflection near the girl
    const float roboFade = sq(clamp01((y / environment.height - 0.825f) / 0.175f));
    const float roboX = x + dx * roboFade;
    const float roboY = environment.height * 1.65f - y + dy * roboFade;

    float roboMask = 0.0f;
    if (y > environment.height * 0.825f) {
        roboMask = sample(source, Character, roboX, roboY);
    }
        
    for (int c = 0; c < 3; ++c) {
        pixel.color[c] = lerp(
            sample(environment, c, rx, ry),
            sample(source, c, roboX, roboY),
            roboMask);
        pixel.color[c] =
            sample(source, c, x, y) * (1 - reflectivity * 0.48f)
            + pixel.color[c] * reflectivity * 0.72f;
    }

    pixel.emission =
        sample(source, Emission, x, y) +
        lerp(sample(environment, Emission, rx, ry),
             sample(source, Emission, roboX, roboY), roboMask) * reflectivity * 0.32f;

    return pixel;
}

} // namespace

void floorReflection(Image& scene, const Image& environment, RandomSeed seed)
{
    Image source = scene;
    Image ripples = noise(scene.width, scene.height, seed, 1.0f);

    auto c = channels(scene);
    auto cc = channels(source);
    auto r = channels(ripples);

    int offset = 0;
    for (int j = 0; j < scene.height; j++) {
        for (int i = 0; i < scene.width; i++) {

            const float reflectivity = cc[Reflectivity][offset];

            if (reflectivity > 0) {
                const float ripple = r[0][offset];

                const Pixel p = shader(source, environment, i, j, reflectivity, ripple);

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
