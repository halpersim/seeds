#version 440 core

in vec3 N;
in vec3 V;
flat in uint instance_id;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 id;

layout(binding = 1, std140) readonly buffer id_buffer{
    ivec4 ids[];
};

layout(binding = 2, std140) readonly buffer owner_index_buffer{
    ivec4 owner_idx[];
};

layout(binding = 5, std140) readonly buffer player_color_buffer{
    vec4 colors[];
};


void main(){
 //   color = vec4(vs_color,1);
 //   id = vec4(ids[instance_id/4][instance_id%4], 0, 0, 1);

    float d = dot(normalize(N), normalize(V));
    vec4 normal_color = vec4(normalize(N) * 0.5 + 0.5, 0);
    float spec_factor = sign(d) * pow(d, 100);
    vec4 owner_color = colors[owner_idx[instance_id >> 2][instance_id & 3]];

    float bw = normal_color.r * 0.3 + normal_color.g * 0.6 + normal_color.b * 0.1;

    //color = normal_color * 0.6 + vec4(vec3(0.1), 0) * spec_factor * 0.1 + owner_color * 0.3;
    color = bw * owner_color;
    color.a = 1.f; 
    id = vec4(ids[instance_id >> 2][instance_id & 3], 0, 0, 1);

    
    /*
    color = vec4(ids[instance_id], 0, 0, 1);
    id = vec4(ids[instance_id], 0, 0, 1);
    */
}