#include "geometry-helpers.h"

#include "logger.h"
#include "vk_helper_functions.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace backpack {

    VkVertexInputBindingDescription GetVertexBindingDescription() {
        // How the vertex data is passed. So through index 0 in the binding array of the pipeline, how large the vertex is and how it should be processed.
        VkVertexInputBindingDescription desc{};
        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return desc;
    }

    std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescription() {
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

        VkVertexInputAttributeDescription desc_3{};
        desc_3.binding = 0;
        desc_3.location = 2;
        desc_3.offset = offsetof(Vertex, texcoord);
        desc_3.format = VK_FORMAT_R32G32_SFLOAT;

        return std::array<VkVertexInputAttributeDescription, 3>{desc_1, desc_2, desc_3};
    }

}

namespace backpack {
    Model3D LoadSingleModel3D(VkDevice device, VkPhysicalDevice phys_device, VkCommandPool cmd_pool, VkQueue device_queue, uint32_t frames_in_flight, const std::vector<backpack::Vertex>& vertices, const std::vector<uint16_t>& indices) {
        VkDeviceSize vertices_size = sizeof(backpack::Vertex) * vertices.size();
        VkDeviceSize indices_size = sizeof(uint16_t) * indices.size();

        Model3D model{};
        VkBuffer staging_buffer;
        VkDeviceMemory staging_memory;

        // Create staging buffer
        CreateBuffer(device, phys_device, vertices_size + indices_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            staging_buffer, staging_memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);

        // Create device buffer
        CreateBuffer(device, phys_device, vertices_size + indices_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            model.gpu_buffer, model.gpu_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr);

        // Map data and copy to GPU
        void* data;
        vkMapMemory(device, staging_memory, 0, vertices_size + indices_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(vertices_size));
        memcpy(static_cast<char*>(data) + vertices_size, indices.data(), static_cast<size_t>(indices_size));
        vkUnmapMemory(device, staging_memory);

        model.index_offset = static_cast<uint32_t>(vertices_size);

        // Do GPU copy operations
        VkCommandBuffer cmd_buffer = BeginSingleTimeCommandBuffer(device, cmd_pool);

        // Copy device memory
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = vertices_size + indices_size;
        vkCmdCopyBuffer(cmd_buffer, staging_buffer, model.gpu_buffer, 1, &copy_region);

        EndSingleTimeCommandBuffer(device, device_queue, cmd_pool, cmd_buffer);

        vkDestroyBuffer(device, staging_buffer, nullptr);
        vkFreeMemory(device, staging_memory, nullptr);

        //// Create UBO's
        //model.ubo_buffer.resize(frames_in_flight);
        //model.ubo_memory.resize(frames_in_flight);
        //model.ubo_mapped_memory.resize(frames_in_flight);

        //VkDeviceSize size = sizeof(UniformBufferObject);
        //for (int i = 0; i < frames_in_flight; i++) {
        //	CreateBuffer(device, phys_device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, model.ubo_buffer[i], model.ubo_memory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //	vkMapMemory(device, model.ubo_memory[i], 0, size, 0, &model.ubo_mapped_memory[i]);
        //}

        return model;
    }

    void DestroyModel(VkDevice device, Model3D* model) {
        if (model->gpu_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, model->gpu_buffer, nullptr);
            model->gpu_buffer = VK_NULL_HANDLE;

        }
        if (model->gpu_memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, model->gpu_memory, nullptr);
            model->gpu_memory = VK_NULL_HANDLE;
        }
    }

    ModelPacked ModelLoader::LoadModels(std::vector<std::string> paths) {
        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;
        tinyobj::LoadObj(&attributes, &shapes, &materials, &err, VIKING_ROOM_M.c_str());

        ModelPacked
        for (auto& shape : shapes) {
            shape.mesh.
        }

    }

    void ModelLoader::LoadModelsToGPU(std::vector<ModelPacked> model_data) {
    }
}
