#version 440 core

out vec2 tc;

void main(){
    vec2[4] vertices = vec2[4](vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, 1));

    gl_Position = vec4(vertices[gl_VertexID], 0.5, 1);
    tc = vertices[gl_VertexID] * 0.5 + 0.5;
}