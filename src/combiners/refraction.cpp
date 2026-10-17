#include "refraction.h"

#include "infra/sample.h"

namespace cyber {

namespace {
struct Pixel {
    Tensor<3> color;
    float emission{};
};

Pixel refractShader(const Image &source, int x, int y, 
    const Pixel &input, float mask, float dx, float dy, float lens, int pixelScale) {
    Pixel p = input;

    Tensor<3> shiftedColor;
    for (int c = 0; c < 3; c++) {
        shiftedColor[c] = sample(source, c, x + dx, y + dy);
    }

    lerp(p.color, shiftedColor, mask * 0.95f);

    const float horizontalEdge = sample(source, Lens, x - 1, y) - sample(source, Lens, x + 1, y);
    const float verticalEdge = sample(source, Lens, x, y - 1) - sample(source, Lens, x, y + 1);
    const float edge = 0.1f * std::max(0.0f, horizontalEdge + verticalEdge);

    const float streak = std::max(0.0f, mask - lens) * 0.025f;

    for (int c = 0; c < 3; c++) {
        p.color[c] += streak;
    }

    Tensor<3> refracted;
    for (int c = 0; c < 3; c++) {
        refracted[c] = sample(source, c,
            source.width * 0.85f - x * 0.3f + dx * 3,
            source.height * 0.58f - y * 0.28f + dy * 3);

        p.color[c] += refracted[c] * lens * 0.012f;
    }

    const float brightness = std::max({ p.color[0], p.color[1], p.color[2] });
    p.emission += mask * std::min(brightness, 3.0f) * 0.08f;

    return p;
}
} // namespace

void refraction(Image& scene, int pixelScale)
{
    Image source = scene;

    auto c = channels(scene);
    auto cc = channels(source);

    int index = 0;
    for (int j = 0; j < source.height; j++) {
        for (int i = 0; i < source.width; i++) {
            const Pixel inputPixel{ {cc[R][index], cc[G][index], cc[B][index]}, cc[Emission][index] };
            const float dx = cc[Dx][index];
            const float dy = cc[Dy][index];
            const float mask = cc[Refract][index];
            const float lens = cc[Lens][index];

            const Pixel p = refractShader(source, i, j, 
                inputPixel, mask, dx, dy, lens, pixelScale);

            c[R][index] = p.color[0];
            c[G][index] = p.color[1];
            c[B][index] = p.color[2];
            c[Emission][index] = p.emission;

            index++;
        }
    }
}

} // cyber
