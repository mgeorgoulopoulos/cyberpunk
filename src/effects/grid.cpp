#include "grid.h"

#include "infra/noise.h"
#include "infra/random_uniform.h"
#include "infra/sample.h"

#include <optional>
#include <stdexcept>

namespace cyber {

namespace {

struct Channels {
    Tensor<3> color;
    float depth{};
    float reflectivity{};
    float emission{};
};

constexpr float vHorizon = 0.595f;

inline float pmod(float x, float m) {
    return std::fmod(std::fmod(x, m) + m, m);
}

std::optional<Channels> gridShader(const Image &scene, float u, float v, 
    const Image &rough, const Image &wetness,
    float speck, int pixelScale) {
    const float worldTile = 3.0f;

    const float uneven = sampleUv(rough, 0, u, v) - 0.5f;
    const float wet = sampleUv(wetness, 0, u, v);

    const float aspect = (float)scene.width / (float)scene.height;

    Channels result;

    v -= vHorizon;

    if (v <= 0) {
        return std::nullopt;
    }

    const float wz = 1.7f / v;
    const float wx = (u - 0.5) * wz * aspect + 0.22 * uneven;

    float dx = pmod(wx, worldTile);
    dx = std::min(dx, worldTile - dx);
    float dz = pmod(wz + 0.16f * uneven, worldTile);
    dz = std::min(dz, worldTile - dz);

    const float lineWidth =
        std::max(wz / scene.height * pixelScale, 0.013f);

    const float xWidth = lineWidth * 0.8f;
    const float zWidth =
        std::max(wz * wz / (1.7f * scene.height) * pixelScale, 0.024f);

    const float xLine = std::exp(-sq(dx / xWidth));
    const float zLine = std::exp(-sq(dz / zWidth));

    float lines = std::max(xLine, zLine);
    lines *= clamp01(v * 10.0f)
        * clamp01((uneven + 0.5f - 0.16f) * 1.5f);

    const float base = 0.009f + 0.027f * wet + 0.01f * speck;
    result.color = Tensor<3>{ {base, base * 0.75f, base * 1.8f} };
    Tensor<3> lineColor{ {0.65f, 0.055f, 0.65f} };
    lineColor *= lines;
    result.color += lineColor;

    result.reflectivity = clamp((wet - 0.36f) * 5.0f, 0.1f, 1.0f);

    result.depth = wz;


    result.emission = lines * 0.5f;

    return result;
}

} // namespace

void grid(Image& scene, int pixelScale)
{
    // Textures
    const Image wetness = noise(scene.width, scene.height, 122, 0.18);
    const Image rough = noise(scene.width, scene.height, 81);
    RandomUniform random(9);

    std::vector<float*> c = channels(scene);
    
    for (int j = 0; j < scene.height; j++) {
        for (int i = 0; i < scene.width; i++) {
            const std::optional<Channels> channels = gridShader(
                scene,
                i / (scene.width - 1.0f),
                j / (scene.height - 1.0f),
                rough, wetness,
                random.next(),
                pixelScale);

            if (channels.has_value() && channels->depth < *c[Depth]) {
                *c[R] = channels->color[R];
                *c[G] = channels->color[G];
                *c[B] = channels->color[B];
                *c[Depth] = channels->depth;
                *c[Reflectivity] = channels->reflectivity;
                *c[Emission] = channels->emission;
            }

            for (auto& ptr : c) {
                ptr++;
            }
        }
    }
}

} // cyber
