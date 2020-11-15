#version 440 core

layout(location = 0) in vec2 pos;

uniform mat3 w2NDS;

out vec2 tc;

void main(){
  //  vec2[4] t = vec2[4](vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));

    gl_Position = vec4((w2NDS * vec3(pos, 1)).xy, 0, 1);
    tc = pos * vec2(1, -1);
}