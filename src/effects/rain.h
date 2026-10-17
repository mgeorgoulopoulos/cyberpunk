#pragma once
#include "infra/types.h"

namespace cyber {
// Render streaks + fat drops -> Dx,Dy,Reflect, Lens channels
void rain(Image& scene, int pixelScale, RandomSeed seed);
}
