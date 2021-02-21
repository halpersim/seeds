#version 440 core

layout (location = 0) in vec2 pos;

uniform mat3 w2NDS;

out vec2 tc;

void main(){
    gl_Position = vec4(w2NDS * vec3(pos, 1), 1);
    tc = vec2(1, -1) * pos;
}