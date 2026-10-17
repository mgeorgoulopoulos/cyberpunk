include(FetchContent)

set(GSL_TEST OFF CACHE BOOL "" FORCE)
set(GSL_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ms_gsl
    URL https://codeload.github.com/microsoft/GSL/tar.gz/refs/tags/v4.2.2
    URL_HASH SHA256=59e2a0a0ea22e8bcf9db2dc4d4bd21212ac6595748295fc27a7e02cf75eac4b5
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(ms_gsl)

# Download verified sources beneath the build directory; no system packages,
# Git checkout command, shell configure, Python or package manager is required.
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(zlib
    URL https://codeload.github.com/madler/zlib/tar.gz/refs/tags/v1.3.2
    URL_HASH SHA256=b99a0b86c0ba9360ec7e78c4f1e43b1cbdf1e6936c8fa0f6835c0cd694a495a1
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OVERRIDE_FIND_PACKAGE)
FetchContent_MakeAvailable(zlib)
# libpng asks find_package(ZLIB): the redirect above and alias below supply
# the target we just built rather than searching for an installed binary.
if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(libpng
    URL https://codeload.github.com/pnggroup/libpng/tar.gz/refs/tags/v1.6.59
    URL_HASH SHA256=2540302a1844ad2b2b501977abecfa850f265f97b78f065a712ab4074a89f5b5
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
# Use the supplied standard pnglibconf.h; no optional awk tool is needed.
set(AWK OFF)
set(SKIP_INSTALL_ALL ON)
FetchContent_MakeAvailable(libpng)
unset(SKIP_INSTALL_ALL)
unset(AWK)

FetchContent_Declare(imago
    URL https://codeload.github.com/jtsiomb/libimago/tar.gz/99f2bfe30b7441251e2d0bc2d72d1e6edd04ae36
    URL_HASH SHA256=a89cbd8e125216b74fd109abd71d14d84b6f7155360cf24e17b5937189f7be4a
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(imago)

# Upstream has a shell configure/Makefile, so supply the small native CMake
# target ourselves. Register PNG only; other codecs and OpenGL are not compiled.
file(WRITE "${imago_BINARY_DIR}/modules.c"
    "int img_register_png(void);\nvoid img_modules_init(void) { img_register_png(); }\n")

# Build-local PNG compatibility fixes: expand palette/tRNS/low-bit grayscale
# and gray-alpha to RGB(A), keeping alpha and 16-bit precision. Reject short
# reads/writes correctly (the callbacks return size_t). Never alter downloaded sources.
file(READ "${imago_SOURCE_DIR}/src/filepng.c" imago_png)
string(REPLACE "png_read_png(png, info, 0, 0);"
    "png_set_expand(png);\n\tpng_set_gray_to_rgb(png);\n\tpng_read_png(png, info, 0, 0);" imago_png "${imago_png}")
string(REPLACE "io->read(data, len, io->uptr) == -1" "io->read(data, len, io->uptr) != len" imago_png "${imago_png}")
string(REPLACE "io->write(data, len, io->uptr) == -1" "io->write(data, len, io->uptr) != len" imago_png "${imago_png}")
# png_write_png already writes IEND; avoid a duplicate trailing end chunk.
string(REPLACE "png_write_end(png, info);" "/* IEND was written by png_write_png. */" imago_png "${imago_png}")
file(WRITE "${imago_BINARY_DIR}/filepng.c" "${imago_png}")
add_library(imago_png STATIC
    "${imago_SOURCE_DIR}/src/imago2.c"
    "${imago_SOURCE_DIR}/src/conv.c"
    "${imago_SOURCE_DIR}/src/quant.c"
    "${imago_SOURCE_DIR}/src/byteord.c"
    "${imago_SOURCE_DIR}/src/ftmodule.c"
    "${imago_BINARY_DIR}/filepng.c"
    "${imago_BINARY_DIR}/modules.c")
target_include_directories(imago_png PUBLIC "${imago_SOURCE_DIR}/src")
target_link_libraries(imago_png PRIVATE png_static)
if(MSVC)
    target_compile_definitions(imago_png PRIVATE _CRT_SECURE_NO_WARNINGS WIN32)
endif()
