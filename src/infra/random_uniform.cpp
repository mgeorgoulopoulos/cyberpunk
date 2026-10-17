#include "random_uniform.h"

#include <cmath>

namespace cyber {
RandomUniform::RandomUniform(RandomSeed seed) : engine(seed) {
}

float RandomUniform::next() {
    return distribution(engine);
}

RandomSeed RandomUniform::nextSeed() {
    return (RandomSeed)engine();
}
} // cyber
