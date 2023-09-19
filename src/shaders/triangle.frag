#version 450

layout(binding = 1) uniform sampler2D tex_sampler;

// Input variables from the vertex shader
layout(location = 0) in vec3 vert_color;
layout(location = 1) in vec2 vert_texcoord;

// Color output
layout(location = 0) out vec4 out_color;

void main (){
    out_color = vec4(mix(texture(tex_sampler, vert_texcoord), vec4(vert_color, 1.0f), 0.5f));
}