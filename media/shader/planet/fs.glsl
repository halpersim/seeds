#version 440 core

in GS_OUT{
    vec3 V;
    vec3 L;
	vec3 N;
	vec3 color;
	flat uint instance_id;
}fs_in;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 id;

/*
uniform	vec3 spec = vec3(1.f);
uniform	vec3 diff = vec3(0.8f, 0.f, 0.8f);
uniform	vec3 amb = vec3(0.2f, 0.f, 0.2f);
uniform int spec_exp = 1<<5;
*/

layout(binding = 3, std140) readonly buffer id_buffer{
    ivec4 ids[];
};

layout(binding = 4, std140) readonly buffer owner_index_buffer{
	ivec4 owner_indices[];
};

layout(binding = 5, std140) readonly buffer player_buffer{
	vec4 player_colors[];
};

void main(){

/*	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);

	vec3 R = -normalize(reflect(L, N));

	float d = max(0.f, dot(N, L));
	float s = pow(max(0.f, dot(V, R)), spec_exp);

	color = vec4(amb + d*diff + s*spec, 1.f);	

	if(length(fs_in.color) < 0.5)
		color = vec4((N+1)/2, 1);
	else
		color = vec4(fs_in.color, 1);*/

	//id = ivec4(1, 0, 0, 0);

	color = vec4((normalize(fs_in.N) * 0.5 + 0.5) * 0.3 + (player_colors[owner_indices[fs_in.instance_id >> 2][fs_in.instance_id & 3]] * 0.4).xyz, 1);
	id = vec4(ids[fs_in.instance_id >> 2][fs_in.instance_id & 3], 0, 0, 0);
}