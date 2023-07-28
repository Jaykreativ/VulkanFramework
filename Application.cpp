#include "Application.h"

#include <chrono>

namespace app {
    GLFWwindow* window;

    const uint32_t WIDTH = 1000;
    const uint32_t HEIGHT = 600;
    const char* TITLE = "RTX Application";

    Shader vertShader = Shader("Shader/shader.vert.spv");
    Shader fragShader = Shader("Shader/shader.frag.spv");

    std::vector<Vertex> vertexArray = {
        Vertex({ 0, -0.5f, 0 }),
        Vertex({ -0.5f, 0.5f, 0 }),
        Vertex({ 0.5f, 0.5f, 0 })
    };
    std::vector<uint32_t> indexArray = {
        0, 1, 2
    };

    UniformBufferObject ubo = {
        {1, 1, 1, 1} //Color
    };
}

int main() {
    initGLFW();
    initVulkan();

    printStats();
    
    float a = 0;

    while (!glfwWindowShouldClose(app::window)) {
        if (a > 1) a = 0;
        a += 0.005;

        app::ubo.color = glm::vec4(a, 1, 0.5, 1);

        updateUniforms();
        drawFrame();

        glfwPollEvents();
    }

    terminateVulkan();
    terminateGLFW();
 }