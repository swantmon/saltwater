
#ifndef __INCLUDE_VS_P3_GLSL__
#define __INCLUDE_VS_P3_GLSL__

layout(location = 0) in vec3 in_Vertex;

layout(location = 0) out vec3 out_WSPosition;
layout(location = 1) out vec3 out_WSNormal;
layout(location = 2) out vec2 out_UV;
layout(location = 3) out mat3 out_WSNormalMatrix;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(in_Vertex.xyz, 1.0f);

    out_WSPosition = in_Vertex;
    
    out_WSNormal = in_Vertex;

    out_UV = vec2(0.0f);

    out_WSNormalMatrix = mat3(0.0f);
}

#endif // __INCLUDE_VS_P3_GLSL__