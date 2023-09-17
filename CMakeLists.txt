cmake_minimum_required(VERSION 3.10)
file(GLOB VulkanFramework_SRC
    "include/*"
    "src/*"
)
add_library(VulkanFramework ${VulkanFramework_SRC})
target_include_directories(
    VulkanFramework PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "$ENV{VULKAN_SDK}/Include"
)