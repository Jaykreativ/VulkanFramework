#pragma once

#include "VulkanRenderer.h"
#include "Shader.h"
#include "Vertex.h"

namespace app {
    extern GLFWwindow* window;

    extern const uint32_t WIDTH;
    extern const uint32_t HEIGHT;
    extern const char* TITLE;

    extern Shader vertShader;
    extern Shader fragShader;

    extern std::vector<Vertex> vertexArray;
    extern std::vector<uint32_t> indexArray;

    extern UniformBufferObject ubo;
}

