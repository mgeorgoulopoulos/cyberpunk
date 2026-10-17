#pragma once
#include "infra/types.h"

#include <cstdint>
#include <random>

namespace cyber {
class RandomUniform {
public:
    explicit RandomUniform(RandomSeed seed);
    float next(); // Uniform in [0, 1).
    RandomSeed nextSeed();

private:
    std::mt19937 engine;
    std::uniform_real_distribution<float> distribution{0.0f, 1.0f};
};
}
