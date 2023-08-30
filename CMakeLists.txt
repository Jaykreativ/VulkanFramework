cmake_minimum_required(VERSION 3.10)
project(VulkanFramework)
file(GLOB VulkanFramework_SRC
    "*.cpp"
    "*.h"
)
add_executable(VulkanFramework ${VulkanFramework_SRC})
add_subdirectory(Dependencies/glfw)
add_subdirectory(Dependencies/glm)
target_link_libraries(VulkanFramework PUBLIC glfw glm $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
target_include_directories(
    VulkanFramework PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/Dependencies/glfw/include"
    "${PROJECT_SOURCE_DIR}/Dependencies/glm/glm"
    "$ENV{VULKAN_SDK}/Include"
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanFramework)