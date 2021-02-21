#version 440 core

layout (quads, fractional_odd_spacing, ccw) in;

in TCS_OUT{
    flat uint instance_id;
}tes_in[];

out TES_OUT{
    vec3 normal;
    float depth;
    flat uint id;
}tes_out;

uniform mat4 vp;
uniform float max_size;

layout(binding = 0, std140) readonly buffer matrix_buffer{
	mat4 mw[];
};

float get_depth(float x, float size){
    if((max_size - size) < 0.1){
        return 0.f;
    }
    return pow(x, 0.8f - 0.7f * (size - 1.f)/(max_size - 1.f)) - 1.f;    
}

vec4 evaluate_patch(vec2 coords){
    mat4 matrix = mw[tes_in[0].instance_id];
    float size = matrix[1][3];
    matrix[0][3] = 0;
    matrix[1][3] = 0;

    float theta = coords.x * 2 * 3.141592;
    float radius = coords.y;
    
    float depth = get_depth(coords.y, size);
    vec2 pos_2d = radius * vec2(cos(theta), sin(theta));
    
    return matrix * vec4(pos_2d.x, depth, pos_2d.y, 1);
}

void main(){
    vec4 world = evaluate_patch(gl_TessCoord.xy);
    vec4 dx = evaluate_patch(gl_TessCoord.xy + vec2(0.01f, 0.f));
    vec4 dy = evaluate_patch(gl_TessCoord.xy + vec2(0.f, 0.01f));

    tes_out.normal = normalize(cross(normalize(dx.xyz - world.xyz), normalize(dy.xyz - world.xyz)));
    tes_out.depth = get_depth(gl_TessCoord.y, mw[tes_in[0].instance_id][1][3]) + 1.f;
    tes_out.id = uint(mw[tes_in[0].instance_id][0][3]);
    
    gl_Position = vp * world;
}

