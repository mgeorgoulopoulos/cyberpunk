#pragma once
#include "infra/types.h"

namespace cyber {
// Render streaks + fat drops -> Dx,Dy,Reflect, Lens channels
void rain(Image& scene, std::uint32_t seed = 19, int pixelScale = 1);
}
