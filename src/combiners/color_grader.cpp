#include "color_grader.h"

#include "infra/sample.h"

#include <stdexcept>

namespace cyber {

namespace {
void validateGrader(const Image &grader) {
    if (grader.height != grader.width * grader.width) {
        throw std::invalid_argument("invalid color grader dimensions");
    }

    if (grader.channels < 3) {
        throw std::invalid_argument("invalid grader channel count");
    }
}

Tensor<3> grade(const Tensor<3> &color, const Image &grader) {
    const int dim = grader.width;

    const float rf = color[0] * (dim - 1);
    const int r1 = std::clamp((int)rf, 0, dim - 1);
    const int r2 = std::min(dim - 1, r1 + 1);
    const float rt = rf - r1;

    const float g0 = clamp01(color[1]) * (dim - 1);
    const float g1 = r1 * dim + g0;
    const float g2 = r2 * dim + g0;

    const float b = clamp01(color[2]) * (dim - 1);

    Tensor<3> c1;
    for (int i = 0; i < 3; i++) {
        c1[i] = sample(grader, i, b, g1);
    }

    Tensor<3> c2;
    for (int i = 0; i < 3; i++) {
        c2[i] = sample(grader, i, b, g2);
    }

    lerp(c1, c2, rt);

    return c1;
}

} // namespace

Image colorGrade(const Image &rgb, const Image &grader) {
    validateGrader(grader);
    if (rgb.channels != 3) {
        throw std::invalid_argument("invalid image channel count");
    }

    Image result = rgb;
    auto c = channels(result);

    for (int j = 0; j < result.height; j++) {
        for (int i = 0; i < result.width; i++) {
            Tensor<3> color = {*c[R], *c[G], *c[B]};
            color = grade(color, grader);
            *c[R]++ = color[0];
            *c[G]++ = color[1];
            *c[B]++ = color[2];
        }
    }

    return result;
}
} // cyber