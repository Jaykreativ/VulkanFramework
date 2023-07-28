#pragma once

#include "glm.hpp"

#define DEBUG true

#define PRINT_PHYSICAL_DEVICES true
#define PRINT_QUEUE_FAMILIES  false
#define PRINT_AVAILABLE_DEVICE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_EXTENSIONS false
#define PRINT_AVAILABLE_INSTANCE_LAYERS false

#define VK_INDEX_OF_USED_PHYSICAL_DEVICE 0
#define VK_PREFERED_QUEUE_FAMILY 0
#define VK_PREFERED_AMOUNT_OF_QUEUES 4
#define VK_USED_SCREENCOLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM //TODO civ

struct UniformBufferObject {
    glm::vec4 color;
};


void initGLFW();

void terminateGLFW();

void initVulkan();

void terminateVulkan();

void printStats();

void updateUniforms();

void drawFrame();
