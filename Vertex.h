#pragma once

#include "glm.hpp"

#include "VulkanUtils.h"

class Vertex {
private:

public:
    glm::vec3 pos;

    Vertex(glm::vec3 pos)
        : pos(pos)
    {}

    static VkVertexInputBindingDescription getInputBindingDescription() {
        VkVertexInputBindingDescription inputBindingDescription;
        inputBindingDescription.binding = 0;
        inputBindingDescription.stride = sizeof(Vertex);
        inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return inputBindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getInputAttributeDescriptions() {
        VkVertexInputAttributeDescription posDescription;
        posDescription.location = 0;
        posDescription.binding = 0;
        posDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        posDescription.offset = offsetof(Vertex, pos);

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions = {
            posDescription
        };

        return inputAttributeDescriptions;
    }
};
