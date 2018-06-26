#ifndef __INCLUDE_VS_GATHERING_GLSL__
#define __INCLUDE_VS_GATHERING_GLSL__

layout(std140, binding = 0) uniform UB0
{
    mat4 m_ProjectionMatrix;
    mat4 m_ViewMatrix;
};

layout(binding = 0) uniform sampler2D ps_PhotonPosition;

layout(location = 0) out vec4 out_NormalizedCoords;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    vec2 UV;

    UV.x = gl_VertexID % 1024;
    UV.y = gl_VertexID / 1024;

    UV /= vec2(1024.0f);

    vec4 PhotonPosition = texture(ps_PhotonPosition, UV);

    if (PhotonPosition.a < 0.0f) return;    

    gl_PointSize = 3.0f;
    gl_Position  = m_ProjectionMatrix * m_ViewMatrix * vec4(PhotonPosition.xyz, 1.0f);

    out_NormalizedCoords = gl_Position / gl_Position.w;
}

#endif // __INCLUDE_VS_GATHERING_GLSL__