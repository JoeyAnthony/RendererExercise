#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace geometry_triangle_helpers {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	const std::vector<Vertex> triangle_vertices{
		{{ 0.0f, -.5f, 0.0f}, { 1.0f, 0.0f, 0.0f }},
		{{ .5f, .5f, 0.0f }, {0.0f, 1.f, 0.0f }},
		{{ -.5f, .5f, 0.0f }, {0.0f, 0.0f, 1.0f }}
	};

	VkVertexInputBindingDescription GetVertexBindingDescription();

	std::array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescription();
};
