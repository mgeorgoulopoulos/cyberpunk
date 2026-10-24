#include "infra/image_io.h"

#include "renders/01_cyberpunk.h"

#include <functional>
#include <format>
#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <exception>

namespace {
struct RenderInfo {
    std::string name;
    std::string filename;
    std::string tagLine;
};

const std::vector<RenderInfo> infos{
    {"Reflections of Tomorrow", "ReflectionsOfTomorrow", "Somewhere, it's still 1985."}};

struct Config {
    cyber::RenderSettings settings = {.width = 3840, .height = 2160, .aaScale = 2, .seed = 147994};
    int renderId = 1;

    void validate() { settings.validate();
        if (renderId < 1 || renderId > (int) infos.size()) {
            throw std::invalid_argument("invalid render id");
        }
    }
};

struct CliOpt {
    std::string opt;
    std::string description;
    std::function<int &(Config &)> refer;
};

// clang-format off
const std::vector<CliOpt> availableOpts{
    {"-w", "Width in pixels", [](Config &c) -> int & { return c.settings.width; }},
    {"-h", "Height in pixels", [](Config &c) -> int & { return c.settings.height; }},
    {"-a", "AA scale (both dimensions; setting '2' here -> 4 samples / pixel) ", [](Config &c) -> int & { return c.settings.aaScale; }},
    {"-s", "Random seed", [](Config &c) -> int & { return c.settings.seed; }},
    {"-r", "Rendered image id", [](Config &c) -> int & { return c.renderId; }}
};
// clang-format on

void printHelp() {
    Config defaultConfig;
    printf("cyberpunk [option value]\n\n");
    printf("Available options:\n");
    for (const CliOpt &opt : availableOpts) {
        printf("\t%s\t%s (default: %d)\n", opt.opt.c_str(), opt.description.c_str(),
               opt.refer(defaultConfig));
    }

    printf("\nAvailable renders:\n");
    for (int i = 0; i < (int)infos.size(); i++) {
        const int renderId = i + 1;
        const RenderInfo &info = infos[i];
        printf("\t%d\t%s - %s\n", renderId, info.name.c_str(), info.tagLine.c_str());
    }
}

Config cli(int argc, char *argv[]) {
    Config result;

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() % 2 != 0) {
        throw std::invalid_argument("invalid CLI argument count");
    }

    for (int i = 0; i < (int)args.size(); i += 2) {
        const std::string &opt = args[i];
        const int arg = std::stoi(args[i + 1]);

        const auto it = std::ranges::find(availableOpts, opt, &CliOpt::opt);
        if (it == availableOpts.end()) {
            throw std::invalid_argument("invalid option: " + opt);
        }

        it->refer(result) = arg;
    }

    return result;
}

} // namespace

int main(int argc, char *argv[]) {
    using namespace cyber;

    Config config;
    try {
        config = cli(argc, argv);
    } catch (const std::exception &e) {
        std::printf("E: %s\n", e.what());
        printHelp();
        return 1;
    }

    try {
        config.validate();
        const RenderInfo &info = infos[config.renderId - 1];
        const std::string output =
            "./" + std::format("{:02}", config.renderId) + "_" + info.filename + ".png";

        const Image image = r01_cyberpunk::render(config.settings);
        saveImage(image, output);
    } catch (const std::exception &e) {
        std::printf("E: %s\n", e.what());
        return 1;
    }

    return 0;
}
