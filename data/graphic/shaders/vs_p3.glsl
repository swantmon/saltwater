
#ifndef __INCLUDE_VS_P3_GLSL__
#define __INCLUDE_VS_P3_GLSL__

layout(location = 0) in vec3 in_Position;

layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec2 out_UV;
layout(location = 3) out mat3 out_WSNormalMatrix;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main(void)
{
    gl_Position = vec4(in_Position.xyz, 1.0f);
    
    out_Normal = in_Position;
}

#endif // __INCLUDE_VS_P3_GLSL__