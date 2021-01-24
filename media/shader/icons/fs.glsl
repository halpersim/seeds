#version 440 core

in vec3 tc;

layout(location = 0) uniform sampler2DArray bitmap;

uniform uint bw;

out vec4 color_out;

void main(){

  if(bw != 0){
    vec4 tex = texture(bitmap, vec3(tc), 0);
    color_out = vec4(tex.x, tex.x, tex.x, tex.y);
  } else {
    color_out = texture(bitmap, vec3(tc), 0);
  }
}