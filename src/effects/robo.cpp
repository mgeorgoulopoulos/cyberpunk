#include "robo.h"

#include "infra/blur.h"
#include "infra/sample.h"

namespace cyber {


namespace {

struct Pixel {
    Tensor<3> color;
    Tensor<3> normal;
    float depth{};
    float reflectivity{};
    float emission{};
    float alpha{};
};

Tensor<3> normalize(const Tensor<3>& t) {
    const float length = std::sqrt(sq(t[0]) + sq(t[1]) + sq(t[2]));
    if (length <= 0.0f) {
        return t;
    }

    Tensor<3> result = t;
    result *= 1.0f / length;
    return result;
}

Pixel roboShader(const Image& environment, const Image& normalMap, 
    const Image &material, const Image &emissive, float u, float v) {
    Pixel pixel;

    pixel.color = Tensor<3>{ 1, 0, 0 };

    pixel.normal[0] = 2.0f * sampleUv(normalMap, 0, u, v) - 1.0f;
    pixel.normal[1] = 2.0f * sampleUv(normalMap, 1, u, v) - 1.0f;
    pixel.normal[2] = 2.0f * sampleUv(normalMap, 2, u, v) - 1.0f;
    pixel.normal = normalize(pixel.normal);

    pixel.depth = 3.2f - 0.3f * pixel.normal[2];

    pixel.alpha = sampleUv(normalMap, 3, u, v);
    pixel.reflectivity = 0.0f;

    // Reflection
    const float ru = u + pixel.normal[0] * 0.15f;
    const float rv = v + pixel.normal[1] * 0.18f;
    
    float ru2 = lerp(u, 0.5f + 0.5f * pixel.normal[0], 0.5f) + 0.29f;
    float rv2 = lerp(v, 0.5f - 0.5f * pixel.normal[1], 0.5f) + 0.05f;

    for (int c = 0; c < 3; c++) {
        const float reflection1 = sampleUv(environment, c, ru, rv);
        const float reflection2 = sampleUv(environment, c, ru2, rv2);

        pixel.color[c] = lerp(reflection1, reflection2, 0.28f);
    }

    const float luminance = pixel.color[0] * 0.21f + pixel.color[1] * 0.72f + pixel.color[2] * 0.07f;
    lerp(pixel.color, Tensor<3>{luminance, luminance, luminance}, 0.45f);

    pixel.color *= 4.0f * sampleUv(material, 3, u, v);

    for (int c = 0; c < 3; c++) {
        pixel.color[c] += 2.0f * sampleUv(material, c, u, v);
        pixel.color[c] += 24.0f * sampleUv(emissive, c, u, v);
    }

    const float brightest = std::max({ pixel.color[0], pixel.color[1], pixel.color[2] });
    pixel.emission = std::max(brightest - 1.0f, 0.0f) * 0.05f;

    return pixel;

}

Image rgb(const Image& image) {
    Image result = image;
    result.channels = 3;
    result.pixels.resize(result.channels * result.width * result.height);

    return result;
}
} // namespace

    void robo(Image& scene, const Image& normalMap, const Image& material, const Image& emissive, int pixelScale) {
        const int height = int(scene.height * 0.77f);
        const int width = int((float)height * normalMap.width / normalMap.height);

        const Image environment = boxBlur(rgb(scene), 2 * pixelScale);

        const int left = (int) (scene.width * 0.070f);
        const int top = (int) (scene.height * 0.13f);

        auto c = channels(scene);
        int offset = left + top * scene.width;

        for (int j = 0; j < height; j++) {
            const float v = j / (height - 1.0f);
            for (int i = 0; i < width; i++) {
                const float u = i / (width - 1.0f);

                const Pixel p = roboShader(environment, normalMap, material, emissive, u, v);

                if (p.alpha > 0.0f && c[Depth][offset] > p.depth) {
                    c[R][offset] = lerp(c[R][offset], p.color[0], p.alpha);
                    c[G][offset] = lerp(c[G][offset], p.color[1], p.alpha);
                    c[B][offset] = lerp(c[B][offset], p.color[2], p.alpha);

                    c[Depth][offset] = p.depth;

                    c[Character][offset] = p.alpha;
                    c[Reflectivity][offset] = p.reflectivity;

                    c[Emission][offset] = p.emission;
                }

                offset++;
            }

            offset += scene.width - width;
        }

    }
} // cyber
