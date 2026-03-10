cmake_minimum_required(VERSION 3.16)
project(ParticleSim LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Fetch dependencies ─────────────────────────────────────────────
include(FetchContent)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm)

# ── GLAD (bundled minimal loader) ──────────────────────────────────
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ── Main executable ────────────────────────────────────────────────
add_executable(ParticleSim
    src/main.cpp
    src/renderer.cpp
    src/particle_system.cpp
    src/spatial_grid.cpp
    src/input_handler.cpp
    src/shader_utils.cpp
)

target_include_directories(ParticleSim PRIVATE
    src
)

target_compile_definitions(ParticleSim PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(ParticleSim PRIVATE
    glad
    glfw
    glm::glm
)

# On Windows, opengl32 is linked through GLFW. On Linux, link GL explicitly.
if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(ParticleSim PRIVATE OpenGL::GL)
endif()

# ── Copy shaders to build directory ────────────────────────────────
add_custom_command(TARGET ParticleSim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:ParticleSim>/shaders"
)

# ── Unit Tests (GoogleTest) ────────────────────────────────────────
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(ParticleTests
    tests/test_particle_system.cpp
    src/particle_system.cpp
    src/spatial_grid.cpp
)

target_include_directories(ParticleTests PRIVATE src)
target_link_libraries(ParticleTests PRIVATE
    glm::glm
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ParticleTests)
