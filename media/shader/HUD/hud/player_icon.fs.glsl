#version 440 core

uniform vec3 color;

layout(location = 0) uniform sampler2D bitmap;

in vec2 tc;

out vec4 color_out;

void main(){
    vec4 tex = texelFetch(bitmap, ivec2(tc), 0);
    color_out = vec4(tex.x * color, tex.y);
  //  color_out = vec4(1);
}