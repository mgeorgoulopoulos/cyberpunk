option(CP_ENABLE_CLANG_FORMAT "Validate clang-format setup for editor use" OFF)

# Visual Studio finds .clang-format through the source directory hierarchy.
# Listing it in the project makes the shared style easy to open and edit.
set(CP_FORMAT_STYLE "${PROJECT_SOURCE_DIR}/.clang-format")
set_source_files_properties("${CP_FORMAT_STYLE}" PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(cyberpunk PRIVATE "${CP_FORMAT_STYLE}")
target_sources(ColorGrader PRIVATE "${CP_FORMAT_STYLE}")
source_group("Formatting" FILES "${CP_FORMAT_STYLE}")

if(NOT CP_ENABLE_CLANG_FORMAT)
    return()
endif()

find_program(CP_CLANG_FORMAT_EXECUTABLE
    NAMES clang-format clang-format-19 clang-format-18 clang-format-17 clang-format-16 clang-format-15
    HINTS "${CMAKE_GENERATOR_INSTANCE}/VC/Tools/Llvm/bin"
          "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Tools/Llvm/bin"
    DOC "clang-format used to validate the editor style"
    REQUIRED)

# Parse the style only. Never reformat source files during configure or build.
execute_process(
    COMMAND "${CP_CLANG_FORMAT_EXECUTABLE}" --style=file
            "--assume-filename=${PROJECT_SOURCE_DIR}/src/main.cpp" --dump-config
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RESULT_VARIABLE CP_FORMAT_RESULT
    OUTPUT_QUIET
    ERROR_VARIABLE CP_FORMAT_ERROR)
if(NOT CP_FORMAT_RESULT EQUAL 0)
    message(FATAL_ERROR "Cannot read .clang-format: ${CP_FORMAT_ERROR}")
endif()
message(STATUS "Editor formatting style validated with ${CP_CLANG_FORMAT_EXECUTABLE}")
