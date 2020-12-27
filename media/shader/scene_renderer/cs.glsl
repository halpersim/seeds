#version 440 core

//wecha algorythmus gnomman wird
//R1 ... 8 threads oarweitn des holbwegs kompliziert o (geht owa mit da intel irgendwie net)
//R2 ... 1 thread moch olls alluar
//R3 ... SIZE * SIZE threads werkln zwi  
#define R3

//folls om R3
//size vam Rond
#define SIZE 7

#ifdef R1
layout(local_size_x = 4, local_size_y = 2) in;
#endif
#ifdef R2
layout(local_size_x = 1) in;
#endif
#ifdef R3
layout(local_size_x = SIZE, local_size_y = SIZE) in; 
#endif

layout(binding = 0, r32f) uniform readonly image2D id_texture;
layout(binding = 1, r32f) uniform writeonly image2D border_texture;

uniform uint border_size;
uniform uint selected_id;

void store(ivec2 p, float v){
    if(gl_LocalInvocationIndex == 0)
        imageStore(border_texture, p, vec4(v));
}

#ifdef R1
shared bool hit[5];

void routine(){
    ivec2 my_point = ivec2(gl_WorkGroupID.xy);
    uint my_id = uint(imageLoad(id_texture, my_point).x);

    if(my_id == selected_id){
        store(my_point, 0.0);
        return;
    }
    
    if(gl_LocalInvocationIndex == 0){
        for(int i=0; i<hit.length; i++)
            hit[i] = false;
    }

    ivec2 advance_out; 
    if(gl_LocalInvocationID.x != 3){
        advance_out = ivec2(gl_LocalInvocationID.x - 1, gl_LocalInvocationID.y * 2 - 1);
    }else{
        advance_out = ivec2(gl_LocalInvocationID.y * 2 - 1, 0);
    }
    int prod = advance_out.x * advance_out.y;
    ivec2 dir_around = prod == 0 ? ivec2(-advance_out.y, advance_out.x) : prod < 0 ? ivec2(0, advance_out.x) : ivec2(-advance_out.y, 0);
    bool diagonal = prod != 0;
    bool finished = false;

    ivec2 v = ivec2(0);
    for(int i=1; i<=border_size; i++){
        v = i * advance_out;
        uint idx = diagonal ? i : 0;
        for(int k=0; k<i; k++){
            if(uint(imageLoad(id_texture, min(ivec2(gl_NumWorkGroups.xy), my_point + v)).x) == selected_id){
                hit[idx] = true;
            }
            idx += diagonal ? -1 : 1;
            v += dir_around;
        }
        barrier();
        for(int j=0; j<hit.length; j++){    
            if(hit[j]){
                store(my_point, 1 - mix(0, 1, length(vec2(i, j))/(border_size*1.414213f)));
                finished = true;
                break;
            }
        }
        if(finished)
            break;
    }

    if(!finished)
        store(my_point, 0.0);
}
#endif


#ifdef R2
void routine(){
    ivec2 my_point = ivec2(gl_WorkGroupID.xy);
    uint my_id = uint(imageLoad(id_texture, my_point).x);

    if(my_id == selected_id){
        imageStore(border_texture, my_point, vec4(0));
    }else{
        float closest = border_size * border_size;
        bool hit = false;

        for(int i=0; i<border_size; i++){
            for(int k=0; k<border_size; k++){
                ivec2 p = ivec2(i-border_size/2, k-border_size/2);

                if(uint(imageLoad(id_texture, min(ivec2(gl_NumWorkGroups.xy), my_point + p)).x) == selected_id && length(p) < closest){
                    closest = length(p);
                    hit = true;
                }
            }
        }
        if(!hit){
            imageStore(border_texture, my_point, vec4(0));
        }else{
            imageStore(border_texture, my_point, vec4(1 - mix(0, 1, closest/((border_size-1)*1.414213f/2))));
        }
    }
}
#endif

#ifdef R3
shared uint closest;

void routine(){
    ivec2 common_point = ivec2(gl_WorkGroupID.xy);
    uint common_id = uint(imageLoad(id_texture, common_point).x);

    if(common_id == selected_id){
        store(common_point, 0.0);
        return;
    }

    if(gl_LocalInvocationIndex == 0)
        closest = 1 << 16;

    ivec2 my_point = ivec2(gl_LocalInvocationID.xy) - (ivec2(gl_WorkGroupSize.xy)-1)/2;
    uint my_id = uint(imageLoad(id_texture, common_point + my_point).x);

    barrier();
    if(my_id == selected_id)
        atomicMin(closest, uint(dot(my_point, my_point)));
    barrier();

    if(gl_LocalInvocationIndex == 0){
        //dat oarsch geil ausschaun, wenn da rond a weng gressa war -> is owa zteia
        //store(common_point, smoothstep(0, 1, (closest < (1 << 16)) ? 1 - sqrt(closest)/((gl_WorkGroupSize.x-1)*1.414213f/2) : 0.0));
        store(common_point, closest < (1 << 16) ? 1 : 0);
    }

}
#endif









void main(){
    routine();
}