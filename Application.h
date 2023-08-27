#pragma once

#define GLM_DEPTH_ZERO_TO_ONE

#include "include/VulkanFramework.h"
#include "Vertex.h"
#include "gtc/matrix_transform.hpp"
#include "glm.hpp"

#define Test true

struct UniformBufferObject {
	glm::vec4 color;
	glm::mat4 transform;
	glm::mat4 view;
	glm::mat4 perspective;
	glm::vec2 viewport;
};