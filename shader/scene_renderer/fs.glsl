#version 440 core
in vec2 tc;

layout(binding = 0) uniform sampler2D color_tex;
layout(binding = 1) uniform sampler2D border_tex;
layout(binding = 2) uniform sampler2D id_texture;

out vec4 color_out;

uniform vec3 color;

void main(){

  float t = texture(border_tex, tc).x;
  vec4 col = texture(color_tex, tc);
  
  
  color_out = mix(col, vec4(color, 1), t);



/*
  vec4 t = texture(tex, tc);

  switch(int(t.x)){
    case 1: color_out =  vec4(0.1, 0.1, 0.9, 1); break;
    case 2: color_out =  vec4(0.4, 1.0, 0.1, 1); break;
    case 3: color_out =  vec4(0.5, 0.5, 0.7, 1); break;
    case 4: color_out =  vec4(0.7, 0.0, 0.4, 1); break;
    case 5: color_out =  vec4(0.9, 0.0, 0.8, 1); break;
    case 6: color_out =  vec4(0.2, 0.4, 0.8, 1); break;
    case 7: color_out =  vec4(0.1, 0.2, 0.8, 1); break;
    case 8: color_out =  vec4(0.7, 0.1, 0.9, 1); break;
    case 9: color_out =  vec4(0.4, 0.9, 0.7, 1); break;
    case 10: color_out = vec4(0.9, 0.3, 0.1, 1); break;
    case 11: color_out = vec4(0.4, 0.0, 0.4, 1); break;
    default: color_out = vec4(0.6, 0.1, 0.9, 1); break;
  } */

//  color_out = vec4(texture(color_tex, tc).xyz, 1);

  color_out = vec4(vec3(texture(border_tex, tc).x), 1);
}