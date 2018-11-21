#ifndef __INCLUDE_VS_GATHERING_GLSL__
#define __INCLUDE_VS_GATHERING_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#ifndef CAUSTIC_MAP_RESOLUTION 
    #define CAUSTIC_MAP_RESOLUTION 1024.0f
#endif

#ifndef PHOTON_RESOLUTION_MULTIPLIER 
    #define PHOTON_RESOLUTION_MULTIPLIER 1.0f
#endif

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UB0
{
    mat4 m_ProjectionMatrix;
    mat4 m_ViewMatrix;
};

layout(binding = 0) uniform sampler2D ps_PhotonPosition;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_NormalizedCoords;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main()
{
    vec2 UV;

    UV.x = float(gl_VertexID % int(CAUSTIC_MAP_RESOLUTION));
    UV.y = float(gl_VertexID / int(CAUSTIC_MAP_RESOLUTION));

    UV /= vec2(CAUSTIC_MAP_RESOLUTION);

    vec4 PhotonPosition = texture(ps_PhotonPosition, UV);

    if (PhotonPosition.a < 0.0f) return;    

    gl_PointSize = 3.0f;
    gl_Position  = m_ProjectionMatrix * vec4(PhotonPosition.xyz, 1.0f);

    out_NormalizedCoords = gl_Position / gl_Position.w;
}

#endif // __INCLUDE_VS_GATHERING_GLSL__