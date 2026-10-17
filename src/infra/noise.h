#pragma once
#include "infra/types.h"

namespace cyber {
/*
 * Signle channel of layered noise.
 */
Image noise(int width, int height, RandomSeed seed, float stretch = 1.0f);
}
