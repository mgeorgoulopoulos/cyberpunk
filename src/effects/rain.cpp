#include "rain.h"

#include "infra/random_uniform.h"

#include <numbers>

// #define RAIN_DEBUG

namespace cyber {
namespace {

struct BoundingBox {
    int left, top, right, bottom;
};

struct Streak {
    BoundingBox bounds;
    float width;
};

struct Pixel {
    float dx{};
    float dy{};
    float refract{};
    float lens{};
};

Pixel rainShader(const Streak &streak, float x, float y) {
    const float slope = 0.18f;

    Pixel p;

    const float top = (float)streak.bounds.top;
    const float left = (float)streak.bounds.left;
    const float length = streak.bounds.bottom - streak.bounds.top - 1.0f;
    const float width = streak.width;

    if (length <= 0 || width <= 0) {
        return p;
    }

    const float v = (y - top) / length;
    if (v < 0.0f || v > 1.0f) {
        return p;
    }

    const float centerX = left + 5.0f + (y - top) * slope;
    const float u = (x - centerX) / width;

    const float mask = std::exp(-u * u * 1.8f) *
                       std::pow(std::max(0.0f, std::sin(std::numbers::pi_v<float> * v)), 0.7f);

    p.dx = mask * u * 5.0f;
    p.dy = mask * 2.5f;
    p.refract = mask;

    return p;
}

Pixel fatDropShader(const BoundingBox &bounds, float x, float y) {
    Pixel p;

    const float cx = (bounds.left + bounds.right) * 0.5f;
    const float cy = (bounds.top + bounds.bottom) * 0.5f;
    const float rx = (bounds.right - bounds.left) * 0.5f;
    const float ry = (bounds.bottom - bounds.top) * 0.5f;

    if (rx <= 0 || ry <= 0) {
        return p;
    }

    const float v = (y - cy) / ry;
    float u = (x - cx) / rx;
    u /= (0.8f + 0.2f * std::tanh(v * 2));

    const float distance = u * u + v * v + 0.05f * std::sin(u * 8 + v * 5);
    const float mask = std::clamp((1 - distance) * 9, 0.0f, 1.0f);
    const float bend = std::sqrt(std::max(0.0f, 1 - distance));

    p.dx = u * rx * 0.9f * bend;
    p.dy = v * ry * 0.9f * bend;
    p.refract = mask;
    p.lens = mask;

    return p;
}

} // namespace

void rain(Image &scene, int pixelScale, RandomSeed seed) {
    // Generate streaks
    RandomUniform random(seed);
    auto uniform = [&](float low, float high) { return lerp(low, high, random.next()); };

    std::vector<Streak> streaks;
    for (int i = 0; i < 1000; i++) {
        float x = uniform(0, (float)scene.width);
        float y = uniform(0, (float)scene.height);
        float length = uniform(10, 48) * scene.height / 720.0f;
        float width = uniform(0.55f, 1.3f) * scene.width / 1280.0f;

        streaks.push_back(Streak{{std::max(0, (int)(x - 5)), std::max(0, (int)y),
                                  std::min(scene.width, (int)(x + length * 0.22f + 5)),
                                  std::min(scene.height, (int)(y + length + 1))},
                                 width});
    }

    auto c = channels(scene);

#ifdef RAIN_DEBUG
    for (int index = 0; index < scene.width * scene.height; index++) {
        c[R][index] = c[G][index] = c[B][index] = 0;
    }
#endif

    for (const Streak &streak : streaks) {
        for (int y = streak.bounds.top; y < streak.bounds.bottom; y++) {
            for (int x = streak.bounds.left; x < streak.bounds.right; x++) {
                const Pixel p = rainShader(streak, x, y);

                const int index = y * scene.width + x;

#ifdef RAIN_DEBUG
                c[R][index] += p.refract;
#endif
                c[Dx][index] += p.dx;
                c[Dy][index] += p.dy;
                c[Refract][index] = std::max(0.0f, std::min(1.0f, c[Refract][index] + p.refract));
                c[Lens][index] += p.lens;
            }
        }
    }

    // Generate fat drops
    std::vector<BoundingBox> drops;
    for (int i = 0; i < 84; i++) {
        float cx = uniform(0, float(scene.width));
        float cy = uniform(0, float(scene.height));
        float radius = uniform(9, 38) * scene.height / 720.0f;
        float aspect = uniform(1.1f, 1.9f);

        drops.push_back({int(cx - radius), int(cy - radius * aspect), int(cx + radius),
                         int(cy + radius * aspect)});
    }

    for (const BoundingBox &drop : drops) {
        for (int y = std::max(0, drop.top); y < std::min(scene.height, drop.bottom); y++) {
            for (int x = std::max(0, drop.left); x < std::min(scene.width, drop.right); x++) {
                const Pixel p = fatDropShader(drop, x, y);
                const int index = y * scene.width + x;

                c[Dx][index] = lerp(c[Dx][index], p.dx, p.lens);
                c[Dy][index] = lerp(c[Dy][index], p.dy, p.lens);
                c[Refract][index] = std::max(c[Refract][index], p.refract);
                c[Lens][index] = std::max(c[Lens][index], p.lens);

#ifdef RAIN_DEBUG
                c[R][index] = c[Refract][index];
#endif
            }
        }
    }
}
}
