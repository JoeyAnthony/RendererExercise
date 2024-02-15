#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan.hpp>

struct Scene
{
	std::vector<glm::mat4> object_transforms;
	VkBuffer object_transforms_ubo;
	VkDeviceMemory scene_memory; // scene_data + object_transforms
};
