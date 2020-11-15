#version 440 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 vp;
uniform vec3 eye;
uniform vec3 light;

flat out vec3 vs_color;
flat out uint instance_id;

layout(binding = 0, std140) readonly buffer mw{
    mat4 mw_pallete[];
};

void main(){
    gl_Position = vp * mw_pallete[gl_InstanceID] * vec4(pos + normal * 0.0, 1);
    vs_color = vec3(normal)/2 + vec3(0.5);
    instance_id = gl_InstanceID;
}