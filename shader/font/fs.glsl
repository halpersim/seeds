#version 440 core

in vec2 tc;

uniform sampler2D bitmap;
uniform vec3 color;

out vec4 color_out;

void main(){

  color_out = vec4(color, texelFetch(bitmap, ivec2(tc), 0));
  //  color_out =  vec4(vec3(texelFetch(bitmap, ivec2(tc), 0).x), 1);
  //  color = vec4(vec3(texture(bitmap,vec2(tc.x, tc.y)).x), 1);
}