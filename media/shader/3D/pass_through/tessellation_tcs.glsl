#version 440 core

layout (vertices = 4) out;

in VS_OUT{
    flat uint instance_id;
}tcs_in[];

out TCS_OUT{
    flat uint instance_id;
}tcs_out[];


void main(void)
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = 32.0;
        gl_TessLevelInner[1] = 32.0;
        gl_TessLevelOuter[0] = 32.0;
        gl_TessLevelOuter[1] = 32.0;
        gl_TessLevelOuter[2] = 32.0;
        gl_TessLevelOuter[3] = 32.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].instance_id = tcs_in[gl_InvocationID].instance_id;
}
