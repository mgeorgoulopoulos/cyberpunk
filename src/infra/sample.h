#pragma once
#include "infra/types.h"

namespace cyber {

/*! Sample a pixel from an image. x, y are in image coordinates; use smooth for smoothstep, otherwise bilinear. */
float sample(const Image& image, int channel, float x, float y, bool smooth = false);

// Helper with [0, 1] addressing.
float sampleUv(const Image& image, int channel, float u, float v, bool smooth = false);

// Returns new resized image with same channel count
Image resize(const Image& source, int w, int h, bool smooth = false);

}
