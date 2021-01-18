#version 440 core

in TES_OUT{
    vec3 normal;
	float depth;
	flat uint id;
}fs_in;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 id;

/*
uniform	vec3 spec = vec3(1.f);
uniform	vec3 diff = vec3(0.8f, 0.f, 0.8f);
uniform	vec3 amb = vec3(0.2f, 0.f, 0.2f);
uniform int spec_exp = 1<<5;
*/
void main(){

	/*
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);

	vec3 R = -normalize(reflect(L, N));

	float d = max(0.f, dot(N, L));
	float s = pow(max(0.f, dot(V, R)), spec_exp);

	color = vec4(amb + d*diff + s*spec, 1.f);	
	*/

	//color = vec4(normalize(fs_in.normal) * 0.5f + 0.5f, 1.f);
	//color = vec4(1.f, 0.2f, 0.9f, 1.f);
	color = vec4(vec3(fs_in.depth), 1.f);
	//id = ivec4(1, 0, 0, 0);
	id = vec4(fs_in.id, 0, 0, 0);
}