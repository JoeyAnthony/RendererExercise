#version 450

// vec3 v_data[3][2] = vec3[3][2](
//     vec3[2](vec3(0, -.5, 0), vec3(1.0, 0, 0)),
//     vec3[2](vec3(.5, .5, 0), vec3(1.0, 0, 0)),
//     vec3[2](vec3(-.5, .5, 0), vec3(1.0, 0, 0))
// );

vec3 vertices[3] = vec3[3](
    vec3(0, -.5, 0),  vec3(.5, .5, 0), vec3(-.5, .5, 0)
);

vec3 colors[3] = vec3[3](
    vec3(1.0, 0, 0), vec3(0, 1.0, 0), vec3(0, 0, 1.0)
);

layout(location = 0) out vec3 vert_color;

void main(){
    gl_Position = vec4(vertices[gl_VertexIndex], 1.0);
    vert_color = colors[gl_VertexIndex];
}