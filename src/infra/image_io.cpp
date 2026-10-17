#include "image_io.h"

#include <imago2.h>
#include <cstdio>
#include <cctype>
#include <gsl/util>
#include <limits>
#include <stdexcept>
#include <string>

namespace cyber {

Image loadImage(const std::filesystem::path& path) {

    const std::string filename = path.string();
    FILE *file = std::fopen(filename.c_str(), "rb");
    if (file == nullptr) {
        throw std::runtime_error("Cannot open PNG: " + path.string());
    }

    auto closeFile = gsl::finally([file] {
        std::fclose(file);
    });

    img_pixmap* decoded = img_create();
    if (decoded == nullptr) {
        throw std::runtime_error("Cannot allocate PNG input");
    }
    auto freeDecoded = gsl::finally([decoded] {
        img_free(decoded);
    });

    if (img_read_file(decoded, file) != 0) {
        throw std::runtime_error("Cannot decode PNG: " + path.string());
    }

    // Convert to float
    if (img_convert(decoded, IMG_FMT_RGBAF) != 0) {
        throw std::runtime_error("Cannot convert PNG to RGBA: " + path.string());
    }

    // Pack and return Image
    const int w = decoded->width;
    const int h = decoded->height;
    if (w <= 0 || h <= 0) {
        throw std::runtime_error("PNG dimensions are invalid: " + path.string());
    }

    Image result(w, h, 4);
    const int pixelCount = w * h;
    const float* rgba = static_cast<const float*>(decoded->pixels);
    
    for (size_t i = 0; i < pixelCount; ++i) {
        for (int c = 0; c < 4; ++c) {
            result.pixels[c * pixelCount + i] = rgba[4 * i + c];
        }
    }

    return result;
}

void saveImage(const Image& image, const std::filesystem::path& path) {
    
    if (!image.validDimensions()) {
        throw std::invalid_argument("Invalid output image dimensions");
    }

    // Interleave for imago
    img_pixmap* interleaved = img_create();
    if (interleaved == nullptr) {
        throw std::runtime_error("Cannot allocate PNG output");
    }

    auto freeInterleaved = gsl::finally([interleaved] {
        img_free(interleaved);
    });

    const auto format = image.channels == 4 ? IMG_FMT_RGBAF : IMG_FMT_RGBF;
    if (img_set_pixels(interleaved, image.width, image.height, format, nullptr) != 0) {
        throw std::runtime_error("Cannot allocate PNG output");
    }

    float* pixels = static_cast<float*>(interleaved->pixels);
    const auto c = channels(image);
    const int pixelCount = image.width * image.height;
    for (int i = 0; i < pixelCount; i++) {
        for (int channel = 0; channel < image.channels; channel++) {
            const float value = c[channel][i];
            pixels[i * image.channels + channel] = clamp01(value);
        }
    }

    // Save as png
    if (img_to_integer(interleaved) != 0) {
        throw std::runtime_error("Cannot convert PNG output to bytes");
    }
    if (img_set_name(interleaved, "output.png") != 0) {
        throw std::runtime_error("Cannot set PNG format");
    }

    // Write file
    const std::string filename = path.string();
    FILE *file = std::fopen(filename.c_str(), "wb");
    if (file == nullptr) {
        throw std::runtime_error("Cannot open output PNG: " + path.string());
    }
    auto closeFile = gsl::finally([file] {
        std::fclose(file);
    });

    if (img_write_file(interleaved, file) != 0 || std::fflush(file) != 0) {
        throw std::runtime_error("Cannot write output PNG: " + path.string());
    }
}

} // namespace cyber

