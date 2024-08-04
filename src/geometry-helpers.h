#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>

const std::string VIKING_ROOM_M = "../model/viking_room.obj";
const std::string VIKING_ROOM_T = "../model/viking_room.png";

struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 view_projection;
};

struct ShaderStorageBuffer {
    glm::vec3 particles{};
};

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 transform;
};

namespace backpack {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texcoord;
    };

    const std::vector<uint32_t> quad_indices{
        1, 0, 2, 1, 2, 3, 1
    };

    const std::vector<uint32_t> indices = {
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
    struct Model3D {
        VkBuffer gpu_buffer;
        VkDeviceMemory gpu_memory;
        VkPipeline pipeline;
        uint32_t index_offset;
        uint32_t index_count;
        //std::vector<VkBuffer> ubo_buffer;
        //std::vector<VkDeviceMemory> ubo_memory;
        //std::vector<void*> ubo_mapped_memory;
    };

    struct ModelPacked {
        //uint32_t size_indices;
        //uint32_t* indices;
        //float* position[3];
        //float* orientation[4];
        //float* texcoord[2];

        std::vector<uint32_t> indices;
        std::vector<glm::vec3> position;
        std::vector<glm::vec3> orientation;
        std::vector<glm::vec2> texcoords;
    };

    struct MeshGeometry {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    //size_t GetModelSize(uint32_t num_indices, uint32_t num_vertices) {
    //    short vertex_components = 3;
    //    short texcoord_components = 2;
    //    size_t size = sizeof(uint32_t) * num_indices;
    //    size += sizeof(float) * vertex_components * num_vertices;
    //    size += sizeof(float) * texcoord_components * num_vertices;
    //    return size;
    //}

    //void AllocateModelPacked(uint32_t num_indices, uint32_t num_vertices)
    //{
    //    //size_t size = GetModelSize(num_indices, num_vertices);
    //    //void* ptr = (size) new;
    //}

    Model3D LoadSingleModel3D(VkDevice device, VkPhysicalDevice phys_device, VkCommandPool cmd_pool, VkQueue device_queue, uint32_t frames_in_flight, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    // Does not destroy uthe pipeline object
    void DestroyModel(VkDevice device, Model3D* model);

    class ModelLoader {
    public:
        MeshGeometry LoadModels(std::vector<std::string> paths);
        void LoadModelsToGPU(std::vector<ModelPacked> model_data);
    };
}
