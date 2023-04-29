#version 450

// Input variables from the vertex shader
layout(location = 0) in vec3 vert_color;

// Color output
layout(location = 0) out vec4 out_color;

void main (){
    out_color = vec4(vert_color, 1.0);
}