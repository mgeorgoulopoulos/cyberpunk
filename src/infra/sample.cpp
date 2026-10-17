#include "sample.h"

#include <cmath>

namespace cyber {

float sample(const Image& image, int channel, float x, float y, bool smooth)
{
    x = std::max(0.0f, std::min(image.width - 1.0f, x));
    y = std::max(0.0f, std::min(image.height - 1.0f, y));
    channel = channel % image.channels;

    const int xi0 = (int)x;
    const int yi0 = (int)y;
    const int xi1 = std::min(xi0 + 1, image.width - 1);
    const int yi1 = std::min(yi0 + 1, image.height - 1);

    const float xt = toSmooth(x - xi0, smooth);
    const float yt = toSmooth(y - yi0, smooth);

    const float* plane = &image.pixels[0] + channel * image.width * image.height;

    const float* scan0 = plane + image.width * yi0;
    const float top = lerp(scan0[xi0], scan0[xi1], xt);

    const float* scan1 = plane + image.width * yi1;
    const float bottom = lerp(scan1[xi0], scan1[xi1], xt);

    return lerp(top, bottom, yt);
}

float sampleUv(const Image& image, int channel, float u, float v, bool smooth) {
    return sample(image, channel, u * (image.width - 1), v * (image.height - 1), smooth);
}

Image resize(const Image& source, int w, int h, bool smooth) {
    Image result(w, h, source.channels);

    float* dst = &result.pixels[0];
    for (int c = 0; c < result.channels; c++) {
        for (int j = 0; j < result.height; j++) {
            const float v = j / (result.height - 1.0f);
            for (int i = 0; i < result.width; i++) {
                const float u = i / (result.width - 1.0f);

                *dst++ = sampleUv(source, c, u, v, smooth);
            }
        }
    }

    return result;
}

} // cyberpunk
