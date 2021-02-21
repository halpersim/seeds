#version 440 core

in vec3 tc;

uniform sampler2DArray bitmap;
uniform vec3 color;

out vec4 color_out;

void main(){

  color_out = vec4(color, texelFetch(bitmap, ivec3(tc), 0));
  //color_out = vec4(vec2(texelFetch(bitmap, ivec3(tc), 0)), 1, 1);
 // color_out = vec4(texelFetch(bitmap, ivec3(tc), 0), 1);

//  color_out = vec4(1);
}