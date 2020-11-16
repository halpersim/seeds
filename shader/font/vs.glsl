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
  //  vec2[4] t = vec2[4](vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));

    gl_Position = vec4((mat3(matrix[gl_InstanceID]) * vec3(pos, 1)).xy, 0, 1);
    tc = vec3(pos * vec2(1, -1), texture_indices[gl_InstanceID >> 2][gl_InstanceID % 4]);
}