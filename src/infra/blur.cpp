#include "blur.h"

#include <stdexcept>

namespace cyber {

namespace {
void sumScan(const float* src, float* dst, int size, int radius) {
    if (size <= 0) {
        return;
    }

    // prime 1st element
    dst[0] = radius * src[0];
    for (int i = 0; i <= radius; i++) {
        dst[0] += src[std::min(i, size - 1)];
    }

    // Running sum
    for (int i = 1; i < size; i++) {
        dst[i] = dst[i - 1] - src[std::max(0, i - radius - 1)] + src[std::min(size-1, i+radius)];
    }
}

void addScan(const float* src, float* dst, int size) {
    for (int i = 0; i < size; i++) {
        dst[i] += src[i];
    }
}

void subScan(const float* src, float* dst, int size) {
    for (int i = 0; i < size; i++) {
        dst[i] -= src[i];
    }
}

void copyScan(const float* src, float* dst, int size) {
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

void verticalSum(const float *src, float *dst, int width, int height, int radius) {
    // prime 1st scan
    for (int i = 0; i < width; i++) {
        dst[i] = radius * src[i];
    }
    for (int r = 0; r <= radius; r++) {
        addScan(src + width * std::min(r, height - 1), dst, width);
    }
    
    // Running sum
    for (int i = 1; i < height; i++) {
        dst += width;
        copyScan(dst - width, dst, width);
        subScan(src + width * std::max(0, i - radius - 1), dst, width);
        addScan(src + width * std::min(height - 1, i + radius), dst, width);
    }

}
} // namespace

Image boxBlur(const Image& image, int radius)
{
    Image horizontal(image.width, image.height, image.channels);

    radius = std::abs(radius);
    if (radius == 0) {
        return horizontal;
    }

    const float* srcScan = &image.pixels[0];
    float* dstScan = &horizontal.pixels[0];
    for (int i = 0; i < horizontal.channels * horizontal.height; i++) {
        sumScan(srcScan, dstScan, horizontal.width, radius);
        srcScan += horizontal.width;
        dstScan += horizontal.width;
    }

    // Vertical
    Image result(image.width, image.height, image.channels);
    const float* srcChannel = &horizontal.pixels[0];
    float* dstChannel = &result.pixels[0];
    for (int i = 0; i < horizontal.channels; i++) {
        verticalSum(srcChannel, dstChannel, horizontal.width, horizontal.height, radius);
        srcChannel += result.width * result.height;
        dstChannel += result.width * result.height;
    }

    // average
    float multiplier = 1.0f / ((2 * radius + 1) * (2 * radius + 1));
    for (float& pixel : result.pixels) {
        pixel *= multiplier;
    }

    return result;
}
} // cyber

