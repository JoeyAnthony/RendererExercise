#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
};

namespace geometry_triangle_helpers {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	const std::vector<uint16_t> quad_indices{
		1, 0, 2, 1, 2, 3, 1
	};

	const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
	};

	const std::vector<Vertex> quad_vertices{
		{{ 1.0f, -1.0f, 0.0f}, { 1.0f, 0.5f, 0.0f }},
		{{ -1.0f, -1.0f, 0.0f}, { 1.0f, 0.5f, 0.0f }},
		{{ 1.0f, 1.0f, 0.0f }, {0.0f, 1.f, 0.3f }},
		{{ -1.0f, 1.0f, 0.0f }, {0.1f, 0.0f, 1.0f }}
	};

	const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
	};

	const std::vector<Vertex> triangle_vertices{
	{{ 0.0f, -.5f, 0.0f}, { 1.0f, 0.5f, 0.0f }},
	{{ .5f, .5f, 0.0f }, {0.0f, 1.f, 0.3f }},
	{{ -.5f, .5f, 0.0f }, {0.1f, 0.0f, 1.0f }}
	};

	VkVertexInputBindingDescription GetVertexBindingDescription();

	std::array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescription();
};
