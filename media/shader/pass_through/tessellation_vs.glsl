#version 440 core

out VS_OUT{
    flat uint instance_id;
}vs_out;

void main(){
    vec2[] vertices = vec2[](
        vec2(1, 0), vec2(1,1), vec2(0, 0), vec2(0, 1)
    );

    gl_Position = vec4(vertices[gl_VertexID], 1, 1);
    vs_out.instance_id = gl_InstanceID;
}