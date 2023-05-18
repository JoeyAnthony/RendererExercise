#include "geometry-helpers.h"

VkVertexInputBindingDescription geometry_triangle_helpers::GetVertexBindingDescription()
{
	// How the vertex data is passed. So through index 0 in the binding array of the pipeline, how large the vertex is and how it should be processed.
	VkVertexInputBindingDescription desc{};
	desc.binding = 0;
	desc.stride = sizeof(Vertex);
	desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return desc;
}

std::array<VkVertexInputAttributeDescription, 2> geometry_triangle_helpers::GetVertexAttributeDescription()
{
	VkVertexInputAttributeDescription desc_1{};
	desc_1.binding = 0;
	desc_1.location = 0;
	desc_1.offset = offsetof(Vertex, position);
	desc_1.format = VK_FORMAT_R32G32B32_SFLOAT;

	VkVertexInputAttributeDescription desc_2{};
	desc_2.binding = 0;
	desc_2.location = 1;
	desc_2.offset = offsetof(Vertex, color);
	desc_2.format = VK_FORMAT_R32G32B32_SFLOAT;

	return std::array<VkVertexInputAttributeDescription, 2>{desc_1, desc_2};
}
