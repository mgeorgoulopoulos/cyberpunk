#include "antialias.h"

#include <stdexcept>

namespace cyber {
Image antialias(const Image& input, int scale)
{
    if (scale == 1) {
        return input;
    }

    if (input.width % 2 != 0 || input.height % 2 != 0) {
        throw std::runtime_error("Antialiased image not a power of 2");
    }

    Image result(input.width / 2, input.height / 2, input.channels);
    const auto source = channels(input);
    auto c = channels(result);

    // Downsample 2x
    for (int channel = 0; channel < input.channels; channel++) {
        for (int j = 0; j < result.height; j++) {
            for (int i = 0; i < result.width; i++) {
                float sum = 0.0f;
                for (int dy = 0; dy < 2; dy++) {
                    for (int dx = 0; dx < 2; dx++) {
                        const int index = (j * 2 + dy) * input.width + i * 2 + dx;
                        sum += source[channel][index];
                    }
                }
                c[channel][j * result.width + i] = sum / 4;
            }
        }
    }

    return antialias(result, scale / 2);
}
} // namespace cyber
