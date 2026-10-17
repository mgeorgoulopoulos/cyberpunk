#pragma once
#include "infra/types.h"

namespace cyber {

// Downsample 2x / average as many times as it takes to achieve scale.
Image antialias(const Image& display, int scale);
}
