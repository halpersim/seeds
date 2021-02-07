#version 440 core

layout(local_size_x = 1) in;


layout(binding = 0, r32f) uniform restrict readonly image2D id_texture;
layout(binding = 1, r32f) uniform restrict image2D border_texture;

uniform uint border_size;
uniform uint selected_id;

uniform uint direction;

void main(){
    int sign = int(direction & 1) * 2 - 1;
    ivec2 dir = ivec2((direction >> 1) * sign, (((~(direction >> 1)) & 1) * sign));

    switch(direction){
        case 0: dir = ivec2(1, 0); break;
        case 1: dir = ivec2(-1, 0); break;
        case 2: dir = ivec2(0, 1); break;
        case 3: dir = ivec2(0, -1); break;
    }

    ivec2 size = imageSize(id_texture);
    ivec2 point;

    //point.x = dir.x != 0 ? ((dir.x * -1 + 1) / 2) * size.x - 1: int(gl_WorkGroupID);
    //point.y = dir.y != 0 ? ((dir.y * -1 + 1) / 2) * size.y - 1: int(gl_WorkGroupID);

    point.x = dir.x != 0 ? (dir.x == -1 ? size.x - 1 : 0) : int(gl_WorkGroupID);
    point.y = dir.y != 0 ? (dir.y == -1 ? size.y - 1 : 0) : int(gl_WorkGroupID);
    
    int last_hit = -1;
    while(point.x >= 0 && point.x < size.x && point.y >= 0 && point.y < size.y){
        int id = int(imageLoad(id_texture, point).x);
        int cur_value = -1;

        if(id == selected_id){
            last_hit = 0;
        } else {
            cur_value = int(imageLoad(border_texture, point).x);
        }

        if(cur_value != -1 && (last_hit == -1 || cur_value < last_hit))
            last_hit = cur_value;

        if(last_hit != -1){
            imageStore(border_texture, point, vec4(last_hit));
            last_hit++;
            
            if(last_hit == border_size)
                last_hit = -1;
        }
        point += dir;
    }
}