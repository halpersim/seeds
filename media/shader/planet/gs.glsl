#version 430 core

layout(triangles)in;
layout(triangle_strip, max_vertices=5) out;

uniform mat4 vp;
uniform vec3 light = vec3(0, 0, 2);
uniform vec3 eye = vec3(0, 0, 2);

in TES_OUT{
    vec3 N;
    flat uint instance_id;
}gs_in[];

out GS_OUT{
	vec3 V;
	vec3 L;
	vec3 N;
    vec3 color;
    flat uint instance_id;
}gs_out;

struct hole{
	vec3 bottom_mid;
	float rad;
	vec3 height;
    int padding;
};

struct data{ 
    float radius;
    float thickness;
    uint start_idx;
    uint end_idx;
};

layout(binding = 0, std140) readonly buffer data_buffer{
    data hole_idx_data[];
};

layout(binding = 1, std140) readonly buffer hole_buffer{
	hole excluded_space[];
};
 
layout(binding = 2, std140) readonly buffer mw{
    mat4 mw_pallet[];
};


void set_output_parameters(vec3 pos, vec3 normal, vec3 color){
    gl_Position = vp * mw_pallet[gs_in[0].instance_id] * vec4(pos, 1.f);

    if(dot(normal, gs_in[0].N) < 0)
        normal = -normal;

    gs_out.L = light - pos;
    gs_out.N = mat3(mw_pallet[gs_in[0].instance_id]) * normal;
    gs_out.V = eye - pos;
    gs_out.color = color;
    gs_out.instance_id = gs_in[0].instance_id;
}


vec3 calc_intersection_proj_line_on_cirlce(vec3 I, vec3 O, hole c){
	vec3 N = normalize(c.height);
	vec3 I_proj = I - N * dot(N, I - c.bottom_mid);
	vec3 O_proj = O - N * dot(N, O - c.bottom_mid);
	vec3 V = normalize(O_proj - I_proj);
	vec3 F = I_proj - c.bottom_mid;
	
	float a = dot(V, V);
	float b = 2 * dot(V, F);
	float cs = dot(F, F) - c.rad * c.rad;
	
	float discriminant = b*b - 4*a*cs;
	
	if(discriminant < 0){
		return vec3(0.f);
	}
	discriminant = sqrt(discriminant);
	float t = (-b + discriminant)/(2*a);
	if(t < 0)
		t = (-b - discriminant)/(2*a);
	
	return I_proj + V * t;
}   

void main(){
    bool emitPrimitive = false;
	uint vertex_inside[3];
	int inside_count = 0;

	for(int i=0; i<3; i++)
		vertex_inside[i] = -1;
		
	for(int k=0; k < gl_in.length(); k++){
		bool broke = false;
		vec4 pos;
	
		for(uint i=hole_idx_data[gs_in[0].instance_id].start_idx; i <= hole_idx_data[gs_in[0].instance_id].end_idx; i++){
			vec3 normal = normalize(excluded_space[i].height);
			float dot_value = dot(normal, gl_in[k].gl_Position.xyz - excluded_space[i].bottom_mid);
			vec3 proj_point = gl_in[k].gl_Position.xyz - normal * dot_value;
			
			if(dot_value > 0 && dot_value < length(excluded_space[i].height) && length(proj_point - excluded_space[i].bottom_mid) < excluded_space[i].rad){
				vertex_inside[k] = i;
				inside_count++;
				break;
			}	
		}
	}

    switch(inside_count){
        case 0:
        {  
            for(int i=0; i<gl_in.length(); i++){
                set_output_parameters(gl_in[i].gl_Position.xyz, gs_in[i].N, vec3(0.f));
                EmitVertex();
            }
            EndPrimitive();
            break;
        }

        case 1:
        {
            uint vertex_to_remove_idx;
            uint space_violated_idx;

            for(vertex_to_remove_idx = 0; vertex_to_remove_idx < 3; vertex_to_remove_idx++)
                if(vertex_inside[vertex_to_remove_idx] != -1){
                    space_violated_idx = vertex_inside[vertex_to_remove_idx];
                    break;
                }
            //skizze sullt drauß ling
            vec3 new_vertices[5];

            uint first_idx;
            uint second_idx;

            switch(vertex_to_remove_idx){
                case 0: first_idx = 1; second_idx = 2; break;
                case 1: first_idx = 2; second_idx = 0; break;
                case 2: first_idx = 0; second_idx = 1; break;
            }

            new_vertices[0] = calc_intersection_proj_line_on_cirlce(gl_in[vertex_to_remove_idx].gl_Position.xyz, gl_in[first_idx].gl_Position.xyz, excluded_space[space_violated_idx]);
			new_vertices[1] = gl_in[first_idx].gl_Position.xyz;
            new_vertices[2] = calc_intersection_proj_line_on_cirlce(gl_in[vertex_to_remove_idx].gl_Position.xyz, (gl_in[first_idx].gl_Position.xyz + gl_in[second_idx].gl_Position.xyz)/2, excluded_space[space_violated_idx]);
            new_vertices[3] = gl_in[second_idx].gl_Position.xyz;
			new_vertices[4] = calc_intersection_proj_line_on_cirlce(gl_in[vertex_to_remove_idx].gl_Position.xyz, gl_in[second_idx].gl_Position.xyz, excluded_space[space_violated_idx]);
            

            vec3 vertex_normals[3];
            vertex_normals[0] = cross(new_vertices[0] - new_vertices[2], new_vertices[1] - new_vertices[2]);
            vertex_normals[1] = cross(new_vertices[1] - new_vertices[2], new_vertices[3] - new_vertices[2]);
            vertex_normals[2] = cross(new_vertices[3] - new_vertices[2], new_vertices[4] - new_vertices[2]);

            uint normal_idx = 0;
            for(int i=0; i<5; i++){
                set_output_parameters(new_vertices[i], vertex_normals[normal_idx], vec3(0.f, 0, 0));//normalize(cross(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz))/2 + vec3(0.5f));

                if(i == 1 || i == 2)
                    normal_idx++;
                EmitVertex();
            }
            EndPrimitive();
            
			break;
        }

        case 2:
        {
            uint vertex_outside_idx;
            uint space_violated_idx;

            for(vertex_outside_idx = 0; vertex_outside_idx<3; vertex_outside_idx++)
                if(vertex_inside[vertex_outside_idx] == -1)
                    break;
            
            space_violated_idx = vertex_inside[(vertex_outside_idx+1)%3];
			
            vec3 new_vertices[3];
            for(int i=0; i<3; i++){
                if(i != vertex_outside_idx){
                    new_vertices[i] = calc_intersection_proj_line_on_cirlce(gl_in[i].gl_Position.xyz, gl_in[vertex_outside_idx].gl_Position.xyz, excluded_space[space_violated_idx]);
                }else{
                    new_vertices[i] = gl_in[i].gl_Position.xyz;
                }
            }

            vec3 normal = cross(new_vertices[1] - new_vertices[0], new_vertices[2] - new_vertices[0]);
            for(int i=0; i<3; i++){
                set_output_parameters(new_vertices[i], normal, vec3(0.f, 0.f, 0.f));
				EmitVertex();
			}
            EndPrimitive();
            break;
        }

        case 3:
        {
            //cull it away
            break;
        }
    }

}