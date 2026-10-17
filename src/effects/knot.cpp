#include "knot.h"

#include <numbers>

namespace cyber {

namespace {

Tensor<3> rotateX(const Tensor<3>& p, float angle) {
    Tensor<3> result;

    result[0] = p[0];
    result[1] = p[1] * std::cos(angle) - p[2] * std::sin(angle);
    result[2] = p[1] * std::sin(angle) + p[2] * std::cos(angle);

    return result;
}

Tensor<3> rotateZ(const Tensor<3>& p, float angle) {
    Tensor<3> result;

    result[2] = p[2];
    result[0] = p[0] * std::cos(angle) - p[1] * std::sin(angle);
    result[1] = p[0] * std::sin(angle) + p[1] * std::cos(angle);

    return result;
}

Tensor<3> knot(float t) {
    const float r = 2.0f + 0.65f * std::cos(3.0f * t);

    Tensor<3> result;
    result[0] = r * std::cos(2.0f * t);
    result[1] = r * std::sin(2.0f * t);
    result[2] = 0.9f * std::sin(3.0f * t);

    return result;
}

Tensor<3> transformKnot(Tensor<3> v, const Image& scene) {
    const float xRot = 0.72f;
    const float zRot = -0.28f;

    v = rotateX(v, xRot);
    v = rotateZ(v, zRot);

    // Projection
    const float torusSize = 0.09f;
    const float torusX = 0.79f;
    const float torusY = 0.26f;
    const float perspectiveDepth = 8.0f;

    const float scale = scene.height * torusSize * perspectiveDepth 
        / (perspectiveDepth + v[2]);

    v[0] = scene.width * torusX + v[0] * scale;
    v[1] = scene.height * torusY + v[1] * scale;

    return v;
}

struct Ball {
    Tensor<3> center;
    float t{};
};

std::vector<Ball> knotPoints(int sampleCount, const Image &scene) {
    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    std::vector<Ball> result;
    result.reserve(sampleCount);

    for (int i = 0; i < sampleCount; i++) {
        const float t = twoPi *i / (float)sampleCount;

        Tensor<3> s = knot(t);
        s = transformKnot(s, scene);

        result.push_back({ s, t });
    }

    return result;
}

struct Pixel {
    Tensor<3> color;
    float depth{};
    float emissive{};
    float reflectivity{};
};

std::optional<Pixel> knotShader(int x, int y, const Ball& ball, float radius) {
    Pixel pixel;

    const float dx = x - ball.center[0];
    const float dy = y - ball.center[1];

    const float d2 = (dx * dx + dy * dy) / (radius * radius);

    if (d2 >= 1.0f) {
        return std::nullopt;
    }

    const float normalZ = std::sqrt(1.0f - d2);

    const float color = 0.5f + 0.5f * std::sin(ball.t * 3.0f + 0.5f);

    pixel.color[0] = 2.2f - 2.0f * color;
    pixel.color[1] = 0.07f + 0.95f * color;
    pixel.color[2] = 2.0f;

    const float light = 0.05f + std::pow(normalZ, 7.0f) * 1.4f;
    pixel.color *= light;

    pixel.depth = 14 + ball.center[2] - normalZ;

    pixel.emissive = 0.3f + std::pow(normalZ, 4.0f) * 2.6f;
    pixel.reflectivity = 0.0f;

    return pixel;
    
}

void blitBall(Image& scene, const Ball& ball, float radius) {
    const std::vector<float*> c = channels(scene);

    const int left = std::max(0, (int) std::floor(ball.center[0] - radius));
    const int right = std::min(scene.width, (int) std::ceil(ball.center[0] + radius) + 1);
    const int top = std::max(0, (int) std::floor(ball.center[1] - radius));
    const int bottom = std::min(scene.height, (int) std::ceil(ball.center[1] + radius) + 1);

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            const std::optional<Pixel> op = knotShader(x, y, ball, radius);
            if (!op.has_value()) {
                continue;
            }
            const Pixel& p = op.value();

            const int offset = x + y * scene.width;

            if (c[Depth][offset] <= p.depth) {
                continue;
            }

            c[R][offset] = p.color[0];
            c[G][offset] = p.color[1];
            c[B][offset] = p.color[2];
            c[Depth][offset] = p.depth;
            c[Emission][offset] = p.emissive;
            c[Reflectivity][offset] = p.reflectivity;
        }
    }
}
} // namespace

void knot(Image& scene) {
    const int ballCount = 2400;
    const float ballRadius = std::max(2.0f, scene.height * 0.013f);

    // Sample knot points
    const std::vector<Ball> balls = knotPoints(ballCount, scene);
    for (const auto& ball : balls) {
        blitBall(scene, ball, ballRadius);
    }
}

} // cyber
