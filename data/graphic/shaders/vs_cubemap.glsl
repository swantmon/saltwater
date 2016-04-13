
#ifndef __INCLUDE_VS_CUBEMAP_GLSL__
#define __INCLUDE_VS_CUBEMAP_GLSL__

layout(row_major, std140, binding = 0) uniform UView
{
    mat4 vs_View;
    mat4 vs_Projection;
};

layout(location = 0) in vec3 in_VertexPosition;

layout(location = 2) out vec3 out_TexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    vec4 SSPosition = vs_Projection * vs_View * vec4(in_VertexPosition.xyz, 1.0f);
    
    gl_Position = SSPosition.xyww;
    
    out_TexCoord = in_VertexPosition;
}

#endif // __INCLUDE_VS_CUBEMAP_GLSL__