#pragma once

#include <iostream>
#include <chrono>

#include "glm.hpp"
#include "VulkanUtils.h"

#include "Shader.h"
#include "Vertex.h"

#define PRINT_PHYSICAL_DEVICES true
#define PRINT_QUEUE_FAMILIES  false
#define PRINT_AVAILABLE_DEVICE_EXTENSIONS true
#define PRINT_AVAILABLE_INSTANCE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_LAYERS false

#define VK_INDEX_OF_USED_PHYSICAL_DEVICE 0
#define VK_PREFERED_QUEUE_FAMILY 0
#define VK_PREFERED_AMOUNT_OF_QUEUES 4
#define VK_USED_SCREENCOLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM //TODO civ

struct UniformBufferObject {
    glm::vec4 color;
};

void initGLFW(GLFWwindow*& window, int width, int height, const char* title);

void terminateGLFW(GLFWwindow* window);

void initVulkan(GLFWwindow* window, uint32_t width, uint32_t height, const char* applicationName, UniformBufferObject &ubo, Shader &vertShader, Shader &fragShader, std::vector<Vertex> &vertexArray, std::vector<uint32_t> &indexArray);

void terminateVulkan(Shader &vertShader, Shader &fragShader);

void printStats();

void updateUniform(UniformBufferObject &ubo);

void drawFrame();
