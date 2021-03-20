#version 440 core

layout (quads, fractional_odd_spacing, cw) in;

in TCS_OUT{
    flat uint instance_id;
}tes_in[];

out TES_OUT{
    vec3 N;
    flat uint instance_id;
}tes_out;

struct data{ 
    float radius;
    float thickness;
    uint start_idx;
    uint end_idx;
};

layout(binding = 0, std140) readonly buffer shape_data_buffer{
    data shape_data[];
};

layout(binding = 6, std140) readonly buffer type_buffer{
    ivec4 types[];
};

vec3[2] evaluate_patch_sphere(in vec2 coord){
    float alpha = coord.x * 2 * 3.1415;
    float theta = coord.y * 3.1415;

    float rad = shape_data[tes_in[0].instance_id].radius;

    vec3[2] ret_value;

    ret_value[1] = vec3(
        cos(alpha) * sin(theta),
        cos(alpha) * cos(theta),
        sin(alpha)
    );
    
    ret_value[0] = ret_value[1] * rad;

    return ret_value;
}

vec3[2] evaluate_patch_torus(in vec2 coord){
    float alpha = coord.x * 2 * 3.1415;
    float theta = coord.y * 2 * 3.1415;

    float rad = shape_data[tes_in[0].instance_id].radius;
    float thickness = shape_data[tes_in[0].instance_id].thickness;

    vec3[2] ret_value;

    ret_value[0] = vec3(
		(rad + thickness * cos(alpha)) * cos(theta),
		thickness * sin(alpha),
		(rad + thickness * cos(alpha)) * sin(theta)
	);

    ret_value[1] = normalize(ret_value[0] - vec3(rad * cos(theta), 0, rad * sin(theta)));
    return ret_value;
}

void main(){
    vec3[2] ret_value;
    
    switch(types[tes_in[0].instance_id >> 2][tes_in[0].instance_id & 3]){
        case 0: ret_value = evaluate_patch_sphere(gl_TessCoord.xy); break;
        case 1: ret_value = evaluate_patch_torus(gl_TessCoord.xy); break;
    }

    tes_out.N = ret_value[1];
    tes_out.instance_id = tes_in[0].instance_id;

    gl_Position = vec4(ret_value[0], 1);
}

