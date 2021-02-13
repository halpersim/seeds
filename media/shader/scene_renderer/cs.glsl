#version 440 core

//should be even
layout(local_size_x = {}) in; //gets injected by the program


layout(binding = 0, r32f) uniform restrict readonly image2D id_texture;
layout(binding = 1, r32f) uniform restrict image2D border_texture;

uniform uint border_size;
uniform uint selected_id;

uniform uint shifted;

void iterate_over_range(uvec2 start, uvec2 size, int dir, uint varying_index);

void main(){
    uvec2 size = imageSize(id_texture);
    uvec2 point = gl_WorkGroupID.xy * gl_WorkGroupSize.x + (shifted & 1) * uvec2(gl_WorkGroupSize.x/2);
    point.y += gl_LocalInvocationIndex;

    iterate_over_range(point, size, 1, 0);
    point.x += gl_WorkGroupSize.x - 1;
    iterate_over_range(point, size, -1, 0);
    barrier();

    point.y -= gl_LocalInvocationIndex;
    point.x += gl_LocalInvocationIndex - (gl_WorkGroupSize.x - 1);
    iterate_over_range(point, size, 1, 1);
    point.y += gl_WorkGroupSize.x - 1;
    iterate_over_range(point, size, -1, 1);
}

void iterate_over_range(uvec2 start, uvec2 size, int dir, uint varying_index){
    int last_hit = -1;
    ivec2 point = ivec2(start);
    
    for(int i=0; i < gl_WorkGroupSize.x && point[varying_index] < size[varying_index] && point[varying_index] >= 0; i++){
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
        
        point[varying_index] += dir;
    }
}