cmake_minimum_required(VERSION 3.20)

project(lumina CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include(CheckCXXCompilerFlag)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
# set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/Lib")
# find_package(Vulkan REQUIRED)

# set(SDL2_DIR "$ENV{VULKAN_SDK}/cmake")
# find_package(SDL2 CONFIG REQUIRED)

if(MSVC)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
endif()

add_executable(lumina
    src/main.cpp
    src/lumina/internal/bvh.cpp
    src/lumina/internal/kdtree.cpp
)