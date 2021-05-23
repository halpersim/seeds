#version 440 core

in vec3 tc;

layout(location = 0) uniform sampler2DArray bitmap;

uniform uint bw;

out vec4 color_out;

void main(){

  vec4 tex = texture(bitmap, vec3(tc), 0);

  if(bw != 0){
    if (tex.y < 0.1) {
      discard;
    }

    color_out = vec4(tex.x, tex.x, tex.x, tex.y);
  } else {
  //  if (tex.a < 0.1) {
  //    discard;
  //  }

    color_out = tex;
  }
}