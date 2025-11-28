#!/bin/sh

set -x # Включаем эхо

this_dir=$(dirname "$0")

add_submodule()
{
    git -C "$this_dir" submodule add $2 $1
    git -C "$this_dir" config -f .gitmodules submodule.$1.shallow true
}

add_submodule third_party/external/glfw/repo https://github.com/glfw/glfw
add_submodule third_party/external/glm/repo https://github.com/g-truc/glm
add_submodule third_party/external/stb/repo https://github.com/nothings/stb
add_submodule third_party/external/vulkan_memory_allocator/repo https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

git -C "$this_dir" submodule update --init --recursive --depth 1
