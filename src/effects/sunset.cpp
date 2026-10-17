#include "sunset.h"

#include "infra/blur.h"
#include "infra/random_uniform.h"
#include "infra/sample.h"

#include <stdexcept>

namespace cyber {

namespace {

struct Channels {
    Tensor<3> color;
    float depth{};
    float emissive{};
};

float sun(float u, float v, int w, int h, int pixelScale) {
    const float radius = 0.143f;
    const float uCenter = 0.60f;
    const float vCenter = 0.49f;

    const float cx = (u - uCenter) * (float) w / h;
    const float cy = v - vCenter;
    const float distance = sqrt(sq(cx) + sq(cy));

    float S = clamp01((radius - distance) * h / (float) pixelScale);

    float streaks = pow(clamp01(sq(cos(clamp01(v - 0.45) * 120)) * 2), 2.7f);
    S *= streaks;

    return S;
}

Channels sunsetShader(Image& scene, float u, float v, float cloudDensity, int pixelScale) {
    const Tensor<3> sunColor{ {5.5f, 1.65f, 0.19f} };

    Channels result;

    const float t = (v - 0.57f) / 0.19f;
    const float glow = exp(-t * t);

    const float ribbons = sin(95 * v + 2 * sin(13 * u)) * sin(37 * v - 9 * u);
    float zigzag = 0.75f + 0.25f * sin(19 * u + 8 * v + sin(41 * u - 14 * v));
    zigzag = pow(zigzag, 0.75);

    const float skyR = 0.006f + glow * (1.15f + 0.12f * ribbons) * zigzag;
    const float skyG = 0.012f + glow * (0.17f + 0.018f * ribbons);
    const float skyB = 0.065f + 0.028f * glow + 0.025f * sq(sin(4 * u + 6 * v));

    result.color[R] = skyR;
    result.color[G] = skyG;
    result.color[B] = skyB;

    // Sun color
    const float sunCoverage = sun(u, v, scene.width, scene.height, pixelScale);
    lerp(result.color, sunColor, sunCoverage);

    // Clouds
    const float q = (v - 0.48f) / 0.15f;
    const float cloudLight = exp(-q * q);
    const float k = cloudLight * (1.0f - cloudDensity);
    const Tensor<3> cloudColor{
        0.009f + 0.30f * k,
        0.012f + 0.045f * k,
        0.035f + 0.027f * k
    };
    lerp(result.color, cloudColor, cloudDensity);

    result.depth = 1000.0f;
    result.emissive = (2.8f * sunCoverage + 0.055f * glow) * (1.0f - cloudDensity);

    return result;
}

Image warp(const Image& source, int pixelScale) {
    Image result(source.width, source.height, 1);

    const float* src = &source.pixels[0];
    float* dst = &result.pixels[0];
    for (int j = 0; j < result.height; j++) {
        const float v = j / (result.height - 1.0f);

        for (int i = 0; i < result.width; i++) {
            const float sourceColor = *src++;

            const float u = i / (result.width - 1.0f);
            const float dx = 25 * pixelScale * sin(23 * v + 7 * sourceColor);
            const float dy = 9 * pixelScale * sin(18 * u);
            
            *dst++ = sample(source, 0, i + dx, j + dy, true);
        }
    }

    return result;
}

Image clouds(int w, int h, int pixelScale) {
    Image result(w, h, 1);

    RandomUniform random(31);

    float weight = 1.0f;
    float totalWeight = 0.0f;
    for (int l = 0; l < 5; l++) {
        Image layer(8 << l, 12 << l, 1);
        for (float& pixel : layer.pixels) {
            pixel = random.next();
        }
        combine(result, resize(layer, w, h, true), 
            [weight](float a, float b) {return a + b * weight; });

        totalWeight += weight;
        weight *= 0.55f;
    }

    for (float& pixel : result.pixels) {
        pixel /= totalWeight;
    }

    result = boxBlur(warp(result, pixelScale), std::max(1.0f, h / 240.0f));

    for (float& pixel : result.pixels) {
        pixel = pow(
            clamp((pixel - 0.40f) * 5.0f, 0.0f, 0.88f),
            1.2f
        );
    }

    return result;
}
} // namespace

void sunset(Image& scene, int pixelScale)
{
    const Image cloudImage = clouds(scene.width, scene.height, pixelScale);

    float* buffer = &scene.pixels[0];
    const float* cloudBuffer = &cloudImage.pixels[0];
    std::vector<float*> c = channels(scene);

    for (int j = 0; j < scene.height; j++) {
        for (int i = 0; i < scene.width; i++) {
            const float cloudDensity = *cloudBuffer++;

            const Channels channels = sunsetShader(scene, 
                i / (scene.width - 1.0f), 
                j / (scene.height - 1.0f),
                cloudDensity,
                pixelScale);

            *c[R]++ = channels.color[R];
            *c[G]++ = channels.color[G];
            *c[B]++ = channels.color[B];
            *c[Depth]++ = channels.depth;
            *c[Emission]++ = channels.emissive;
        }
    }
}
}
