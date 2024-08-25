struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;
};

// Define layout std140 which determines the buffer size
// This one is readonly for reading out data
layout(std140, binding = 1) readonly buffer ParticleSSBOIn {
    Particle particlesIn[];
};

// This one can write to memory
layout(std140, binding = 2) buffer ParticleSSBOOut {
    Particle particlesOut[];
};

void main(){

}
