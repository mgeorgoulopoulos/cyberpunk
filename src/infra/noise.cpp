#include "noise.h"

#include "infra/random_uniform.h"
#include "infra/sample.h"
#include <stdexcept>

namespace cyber {

void addWeighed(const Image& src, Image& dstImage, float weight) {
    float* dst = &dstImage.pixels[0];
    for (int j = 0; j < dstImage.height; j++) {
        for (int i = 0; i < dstImage.width; i++) {
            *dst++ += sampleUv(src, 0, i / (dstImage.width - 1.0f), j / (dstImage.height - 1.0f), true) * weight;
        }
    }
}

Image noise(int width, int height, RandomSeed seed, float stretch)
{
    stretch = std::abs(stretch);

    Image result(width, height, 1);
    RandomUniform random(seed);

    float totalWeight = 0.0f;
    for (int octave = 0; octave < 6; octave++) {
        const int w = std::max(2, (int)((6 << octave) / stretch));
        const int h = 4 << octave;
        
        Image octaveImage(w, h, 1);
        float* dst = &octaveImage.pixels[0];
        for (int i = 0; i < w * h; i++) {
            *dst++ = random.next();
        }

        const float weight = std::pow(0.52f, (float) octave);
        addWeighed(octaveImage, result, weight);
        totalWeight += weight;
    }

    const float multiplier = 1.0f / totalWeight;
    for (float& pixel : result.pixels) {
        pixel *= multiplier;
    }

    return result;
}
} // cyber
