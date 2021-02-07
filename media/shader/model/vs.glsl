#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 vp;
uniform vec3 eye;
uniform vec3 light;

out vec3 N;
out vec3 V;
flat out uint instance_id;

layout(binding = 0, std140) readonly buffer mw{
    mat4 mw_pallete[];
};

void main(){
    gl_Position = vp * mw_pallete[gl_InstanceID] * vec4(pos, 1);
    //N = (mw_pallete[gl_InstanceID] * vec4(normal, 1)).xyz;
    N = normal;
    V = eye - gl_Position.xyz;
    instance_id = gl_InstanceID;
}