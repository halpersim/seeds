#version 440 core

layout(local_size_x = 1) in;


layout(binding = 0, r32f) uniform restrict readonly image2D id_texture;
layout(binding = 1, r32f) uniform restrict image2D border_texture;

uniform uint border_size;
uniform uint selected_id;

uniform uint direction;

void main(){
    int sign = (direction & 1) * 2 - 1;
    ivec2 direction = ivec2((direction >> 1) * sign, (((~(direction >> 1)) & 1) * sign);

    ivec2 size = imageSize(id_texture);
    ivec2 point;

    point.x = direction.x != 0 ? (direction.x * -0.5 + 0.5) * size.x : gl_WorkGroupID;
    point.y = direction.y != 0 ? (direction.y * -0.5 + 0.5) * size.y : gl_WorkGroupID;

    int last_hit = -1;
    while(point.x >= 0 && point.x < size.x && point.y >= 0 && point.y < size.y){
        uint id = int(imageLoad(id_texture, point).x);

        if(id == selected_id){
            last_hit = 0;
        }




        if(last_hit != -1)
            last_hit++;
        
        point += direction;
    }
}