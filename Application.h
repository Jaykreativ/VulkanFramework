#pragma once

#include "VulkanRenderer.h"
#include "Vertex.h"

#define Test true

struct UniformBufferObject {
	glm::vec4 color;
	glm::mat4 transform;
	glm::mat4 view;
	glm::mat4 perspective;
	glm::vec2 viewport;
};