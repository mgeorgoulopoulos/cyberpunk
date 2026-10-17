#pragma once
#include <cstdint>
#include <random>

namespace cyber {
class RandomUniform {
public:
    explicit RandomUniform(std::uint32_t seed);
    float next(); // Uniform in [0, 1).

private:
    std::mt19937 engine;
    std::uniform_real_distribution<float> distribution{0.0f, 1.0f};
};
}