#version 440 core

flat in vec3 vs_color;
flat in uint instance_id;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 id;

layout(binding = 1, std140) readonly buffer id_buffer{
    ivec4 ids[];
};

void main(){
 //   color = vec4(vs_color,1);
 //   id = vec4(ids[instance_id/4][instance_id%4], 0, 0, 1);

    color = vec4(vs_color, 1);
    id = vec4(ids[instance_id/4][instance_id%4], 0, 0, 1);

    
    /*
    color = vec4(ids[instance_id], 0, 0, 1);
    id = vec4(ids[instance_id], 0, 0, 1);
    */
}