#pragma once
#include "infra/types.h"

namespace cyber {
/*
 * Signle channel of layered noise.
 */
Image noise(int width, int height, std::uint32_t seed = 7, float stretch = 1.0f);
}
