#include "infra/image_io.h"

#include "renders/01_cyberpunk.h"

#include <cstdio>
#include <exception>

int main(int argc, char *argv[]) {
	using namespace cyber;

	const std::filesystem::path output{"./0101_CyberPunk.png"};

	RenderSettings settings {
		.width = 1280,
		.height = 720,
		.aa_scale = 2,
		.seed = 19
	};
	
	try {
		settings.validate();
		const Image image = r01_cyberpunk::render(settings);
		saveImage(image, output);
	}
	catch (const std::exception& e) {
		std::printf("E: %s\n", e.what());
		return 1;
	}
	
	return 0;
}
