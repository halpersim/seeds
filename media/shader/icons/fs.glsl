#version 440 core

in vec3 tc;

layout(location = 0) uniform sampler2DArray bitmap;

out vec4 color_out;

void main(){
  color_out = texture(bitmap, vec3(tc), 0);
}