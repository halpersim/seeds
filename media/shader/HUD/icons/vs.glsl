#version 440 core

layout(location = 0) in vec2 pos;

layout(binding = 0, std140) uniform texture_index_buffer{
    ivec4 texture_indices[];
};

layout(binding = 1, std140) uniform matrix_buffer{
    mat4 matrix[];
};

out vec3 tc;

void main(){
    gl_Position = vec4((mat3(matrix[gl_InstanceID]) * vec3(pos, 1)).xy, -1, 1);
    tc = vec3(pos, texture_indices[gl_InstanceID >> 2][gl_InstanceID & 3]);
}