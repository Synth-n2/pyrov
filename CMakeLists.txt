cmake_minimum_required(VERSION 3.20)
project(rov_sim LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master # Usually latest 5.5 is fine, or specify a tag like "5.0"
)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)

# Fetch math-oriented dependencies
FetchContent_MakeAvailable(raylib glm)

# Executable
add_executable(rov_sim 
    src/main.cpp 
    src/ROV.cpp 
    src/Props.cpp
)
target_include_directories(rov_sim PRIVATE include)
target_compile_definitions(rov_sim PRIVATE GLM_ENABLE_EXPERIMENTAL)

# Link
target_link_libraries(rov_sim PRIVATE raylib glm::glm)

# Compiler flags for MinGW
if (MINGW)
    target_link_options(rov_sim PRIVATE -static -static-libgcc -static-libstdc++)
endif()
