#pragma once
#include "infra/types.h"

namespace cyber {
// Render reflections
void floorReflection(Image& scene, const Image& environment, RandomSeed seed);
}
