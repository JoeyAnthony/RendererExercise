#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_texcoord;
layout(location = 0) out vec3 vert_color;
layout(location = 1) out vec2 vert_texcoord;

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(push_constant) uniform constants{
    vec4 data;
    mat4 transform;
} push_constants;

void main(){
    gl_Position = push_constants.transform * vec4(in_position, 1.0);
    vert_color = in_color;
    vert_texcoord = in_texcoord;
}