cmake_minimum_required(VERSION 3.25)
project(Cyberpunk LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(cmake/ImageDependencies.cmake)

set(RENDER_SOURCES
    src/infra/image_io.cpp
    src/infra/sample.cpp
    src/infra/blur.cpp
    src/infra/random_uniform.cpp
    src/infra/noise.cpp
    src/effects/sunset.cpp
    src/effects/grid.cpp
    src/effects/knot.cpp
    src/effects/robo.cpp
    src/effects/rain.cpp
    src/combiners/floor_reflection.cpp
    src/combiners/refraction.cpp
    src/combiners/bloom.cpp
    src/combiners/tone.cpp
    src/combiners/color_grader.cpp
    src/combiners/antialias.cpp
    src/renders/01_cyberpunk.cpp
)
add_executable(cyberpunk src/main.cpp ${RENDER_SOURCES})
target_include_directories(cyberpunk PRIVATE src)
target_link_libraries(cyberpunk PRIVATE imago_png Microsoft.GSL::GSL)
if(MSVC)
    target_compile_options(cyberpunk PRIVATE /W4 /permissive- /fp:precise /utf-8 /wd4244 /wd4267 /wd4305)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT cyberpunk)
    set_target_properties(cyberpunk PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS ""
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
else()
    target_compile_options(cyberpunk PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(ColorGrader
    tools/color_grader/ColorGraderMain.cpp
    src/infra/image_io.cpp
    src/infra/image_io.h
    src/infra/types.h)
target_include_directories(ColorGrader PRIVATE src)
target_link_libraries(ColorGrader PRIVATE imago_png Microsoft.GSL::GSL)
set_target_properties(ColorGrader PROPERTIES
    OUTPUT_NAME "ColorGrader"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
if(MSVC)
    target_compile_options(ColorGrader PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(ColorGrader PRIVATE -Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE API_HEADERS CONFIGURE_DEPENDS "src/*.h")
target_sources(cyberpunk PRIVATE ${API_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source"
    FILES src/main.cpp ${RENDER_SOURCES} ${API_HEADERS})
include(cmake/Formatting.cmake)
