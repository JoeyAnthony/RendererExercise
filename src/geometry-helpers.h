#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

constexpr std::string VIKING_ROOM_M = "./models/viking_room.obj";
constexpr std::string VIKING_ROOM_T = "./models/viking_room.obj";

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 view_projection;
};

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 transform;
};

namespace backpack {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texcoord;
	};

	const std::vector<uint16_t> quad_indices{
		1, 0, 2, 1, 2, 3, 1
	};

	const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
	};

	const std::vector<Vertex> quad_vertices{
		{{  1.0f, -1.0f, 0.0f }, { 1.0f, 0.5f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 1.0f, 0.5f, 0.0f }, { 0.0f, 0.0f }},
		{{  1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.3f }, { 1.0f, 1.0f }},
		{{ -1.0f,  1.0f, 0.0f }, { 0.1f, 0.0f, 1.0f }, { 0.0f, 1.0f }}
	};

	const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0,0}},
	{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1,0}},
	{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1,1}},
	{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0,1	}}
	};

	const std::vector<Vertex> triangle_vertices{
	{{ 0.0f, -.5f, 0.0f}, { 1.0f, 0.5f, 0.0f }},
	{{ .5f, .5f, 0.0f }, {0.0f, 1.f, 0.3f }},
	{{ -.5f, .5f, 0.0f }, {0.1f, 0.0f, 1.0f }}
	};

	VkVertexInputBindingDescription GetVertexBindingDescription();

	std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescription();

}

namespace backpack {
	struct Model3D
	{
		VkBuffer gpu_buffer;
		VkDeviceMemory gpu_memory;
		VkPipeline pipeline;
		uint32_t index_offset;
		//std::vector<VkBuffer> ubo_buffer;
		//std::vector<VkDeviceMemory> ubo_memory;
		//std::vector<void*> ubo_mapped_memory;
	};

	struct ModePacked
	{
		// 
		float data[3][3];
	};

	struct Model3DLoadData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	Model3D LoadSingleModel3D(VkDevice device, VkPhysicalDevice phys_device, VkCommandPool cmd_pool, VkQueue device_queue, uint32_t frames_in_flight, const std::vector<geometry_triangle_helpers::Vertex>& vertices, const std::vector<uint16_t>& indices);

	// Does not destroy uthe pipeline object
	void DestroyModel(VkDevice device, Model3D* model);

	class ModelLoader {
		void LoadModels(std::vector<std::string> paths);
		void LoadModelsToGPU(std::vector<Model3DLoadData> model_data);
	};
}
