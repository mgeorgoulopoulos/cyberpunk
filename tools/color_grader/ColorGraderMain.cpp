#include "infra/image_io.h"

constexpr int dim = 16;

int main(int argc, char *argv[]) {
    using namespace cyber;

    Image result(dim, dim * dim, 3);
    auto c = channels(result);

    for (int r = 0; r < dim; r++) {
        for (int g = 0; g < dim; g++) {
            for (int b = 0; b < dim; b++) {
                *c[R]++ = r / (dim - 1.0f);
                *c[G]++ = g / (dim - 1.0f);
                *c[B]++ = b / (dim - 1.0f);
            }
        }
    }
    try {
        saveImage(result, "./color_grader.png");
    } catch (const std::exception &e) {
        std::printf("E: %s\n", e.what());
        return 1;
    }

    return 0;
}
