cmake_minimum_required(VERSION 3.10)
add_library(VulkanFramework VulkanFramework.cpp VulkanUtils.cpp)
target_include_directories(
    VulkanFramework PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
)