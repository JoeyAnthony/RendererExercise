#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

vec3 vertices[3] = vec3[3](
    vec3(0, -.5, 0),  vec3(.5, .5, 0), vec3(-.5, .5, 0)
);

vec3 colors[3] = vec3[3](
    vec3(1.0, 0, 0), vec3(0, 1.0, 0), vec3(0, 0, 1.0)
);

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 0) out vec3 vert_color;

void main(){
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(in_position, 1.0);
    vert_color = in_color;
}