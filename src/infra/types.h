#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <functional>
#include <cstdint>
#include <vector>
#include <stdexcept>

namespace cyber {

template<size_t... D>
struct Tensor {
    std::array<float, (D * ...)> x{};

    template<typename... I> requires (sizeof...(D) == sizeof...(I))
        float& operator[](I... idx) {
        size_t finalIndex = 0;
        size_t currentDim = 0;
        ((finalIndex = finalIndex * dims[currentDim++] + idx), ...);
        return x[finalIndex];
    }

    template<typename... I> requires (sizeof...(D) == sizeof...(I))
        const float& operator[](I... idx) const {
        size_t finalIndex = 0;
        size_t currentDim = 0;
        ((finalIndex = finalIndex * dims[currentDim++] + idx), ...);
        return x[finalIndex];
    }

    Tensor& operator+=(const Tensor& b) { for (size_t i = 0; i < x.size(); ++i) x[i] += b.x[i]; return *this; }
    Tensor& operator-=(const Tensor& b) { for (size_t i = 0; i < x.size(); ++i) x[i] -= b.x[i]; return *this; }
    Tensor& operator*=(float s) { for (auto& v : x) v *= s; return *this; }

private:
    static constexpr std::array dims{ D... };
};

template<class T, class F>
void apply(T& tensor, F f) {
    for (auto& x : tensor.x) {
        x = f(x);
    }
}

template<class T, class F>
void combine(T& a, const T& b, F f) {
    for (size_t i = 0; i < a.x.size(); ++i)
        a.x[i] = f(a.x[i], b.x[i]);
}

// An image consists of N channels placed one after the other.
struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<float> pixels;

    Image() = default;
    Image(int width, int height, int channels) : width(width), height(height), channels(channels), pixels(width * height * channels) {}

    bool validDimensions() const {
        if (width <= 0 || height <= 0) {
            return false;
        }
        if (width * height * channels != (int)pixels.size()) {
            return false;
        }

        return true;
    }
};

inline std::vector<float*> channels(Image& image) {
    std::vector<float*> result;
    result.reserve(image.channels);

    float* buffer = &image.pixels[0];
    const int pixelsPerChannel = image.width * image.height;

    for (int i = 0; i < image.channels; i++) {
        result.push_back(buffer);
        buffer += pixelsPerChannel;
    }

    return result;
}

inline std::vector<const float*> channels(const Image& image) {
    std::vector<const float*> result;
    result.reserve(image.channels);

    const float* buffer = image.pixels.data();
    const int pixelsPerChannel = image.width * image.height;

    for (int i = 0; i < image.channels; i++) {
        result.push_back(buffer);
        buffer += pixelsPerChannel;
    }

    return result;
}

// Apply f to the images and store result to 'a'. Same image dimensions or black output. Source channels repeat if fewer.
template <class F>
inline void combine(Image& a, const Image& b, F f) {
    if (a.width != b.width || a.height != b.height) {
        for (float& pixel : a.pixels) {
            pixel = 0.0f;
        }

        return;
    }

    const int pixelsPerChannel = a.width * a.height;
    float* dst = &a.pixels[0];
    for (int channel = 0; channel < a.channels; channel++) {
        const int sourceChannel = channel % b.channels;
        const float* src = &b.pixels[0] + sourceChannel * pixelsPerChannel;

        for (int i = 0; i < pixelsPerChannel; i++) {
            *dst = f(*dst, *src);
            src++;
            dst++;
        }
    }
}

inline void copy(Image& a, const Image& b) {
    combine(a, b, [](float a, float b) {return b; });
}

inline void add(Image& a, const Image& b) {
    combine(a, b, [](float a, float b) {return a + b; });
}

enum Channel { R, G, B, Depth, Emission, Chrome, Dx, Dy, Refract,
               Character, Reflectivity, Lens, ChannelCount };

struct RenderSettings {
    int width = 1280;
    int height = 720;
    int aa_scale = 2;
    std::uint32_t seed = 19;

    void validate() const {
        if (width < 64 || height < 64 || (aa_scale % 2 != 0)) {
            throw std::invalid_argument("Invalid render settings");
        }
    }
};

inline float toSmooth(float t, bool smooth) {
    return smooth ? t * t * (3 - 2 * t) : t;
}

inline float lerp(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

template<class T>
void lerp(T& a, const T& b, float t) {
    auto f = [t](float a, float b) {return lerp(a, b, t); };
    combine(a, b, f);
}

inline float sq(float f) {
    return f * f;
}

inline float clamp(float x, float from, float to) {
    return std::max(from, std::min(to, x));
}

inline float clamp01(float x) {
    return clamp(x, 0, 1);
}

} // namespace cyber

