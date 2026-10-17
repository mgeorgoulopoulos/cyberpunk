#include "random_uniform.h"

#include <cmath>

namespace cyber {
RandomUniform::RandomUniform(std::uint32_t seed) : engine(seed) {}

float RandomUniform::next() {
    return distribution(engine);
}
} // cyber