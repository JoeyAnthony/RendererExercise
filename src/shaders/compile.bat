:: The file extension tells glslc for which stage to compile, otherwise pass it as an argument.
glslc.exe .\triangle.vert -o .\v_triangle.spv
glslc.exe .\triangle.frag -o .\f_triangle.spv
glslc.exe .\particle.comp -o .\c_particle.spv