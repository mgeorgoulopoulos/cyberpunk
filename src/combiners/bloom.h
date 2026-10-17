#pragma once
#include "infra/types.h"

namespace cyber {

// rgb & enussuve -> blur times amount.
Image bloom(const Image& scene, float amount, int pixelScale = 1);
}

