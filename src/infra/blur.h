#pragma once
#include "infra/types.h"

namespace cyber {
/*
 * Box blur. All channels affected.
 */
Image boxBlur(const Image& image, int radius);
}
