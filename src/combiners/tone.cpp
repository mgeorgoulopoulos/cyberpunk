#include "tone.h"

#include <algorithm>
#include <cmath>

namespace cyber {

namespace {
float luminance(float r, float g, float b) {
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

float averageLuminance(const Image& hdr) {
    const float epsilon = 0.0001f;
    const int pixelCount = hdr.width * hdr.height;
    if (pixelCount == 0) {
        return epsilon;
    }

    const auto source = channels(hdr);

    double logSum = 0.0;
    for (int i = 0; i < pixelCount; i++) {
        const float r = std::max(source[R][i], 0.0f);
        const float g = std::max(source[G][i], 0.0f);
        const float b = std::max(source[B][i], 0.0f);
        logSum += std::log(epsilon + luminance(r, g, b));
    }

    return (float)std::exp(logSum / pixelCount);
}

float exposureValue(float averageLuminance, float key) {
    return key / averageLuminance;
}
} // namespace

Image tone(const Image& hdr)
{
    const float key = 0.18f;

    Image result(hdr.width, hdr.height, 3);
    const int pixelCount = hdr.width * hdr.height;
    if (pixelCount == 0) {
        return result;
    }

    const auto source = channels(hdr);
    auto c = channels(result);
    const float exposure = exposureValue(averageLuminance(hdr), key);

    for (int i = 0; i < pixelCount; i++) {
        const float r = std::max(source[R][i], 0.0f);
        const float g = std::max(source[G][i], 0.0f);
        const float b = std::max(source[B][i], 0.0f);

        // L / (1 + L)
        const float exposed = luminance(r, g, b) * exposure;
        const float gain = exposure / (1.0f + exposed);
        const float power = 1.0f / 2.2f;

        c[R][i] = std::pow(clamp01(r * gain), power);
        c[G][i] = std::pow(clamp01(g * gain), power);
        c[B][i] = std::pow(clamp01(b * gain), power);
    }

    for (float& v : result.pixels) {
        v = toSmooth(v, true);
    }

    return result;
}   
} // namespace cyber
