#version 440 core

uniform vec3 color;

layout(location = 0) out vec4 color_out;

void main(){
  color_out = vec4(color, 1);
}