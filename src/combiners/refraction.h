#pragma once
#include "infra/types.h"

namespace cyber {
// Add refractions to color, emission from Dx,Dy,Refract, Lens channels.
void refraction(Image& scene, int pixelScale = 1);
}
