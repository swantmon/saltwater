#ifndef __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__
#define __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__

// -----------------------------------------------------------------------------
// Original approach and comments:
// "Interactive image-space techniques for approximating caustics"
// Chris Wyman, Scott Davis (2006)
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#ifndef CAUSTIC_MAP_RESOLUTION 
    #define CAUSTIC_MAP_RESOLUTION 1024.0f
#endif

#ifndef DISCARD_TIR 
    #define DISCARD_TIR true
#endif

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB3
{
    mat4 ps_LightProjectionMatrix;
    mat4 ps_LightViewMatrix;
    vec4 ps_LightColor;
    uint ps_ExposureHistoryIndex;
};

layout(std140, binding = 4) uniform UB4
{
    vec4 ps_RefractionIndices;
    vec4 ps_DepthLinearization;
};

layout(binding = 0) uniform sampler2D ps_RefractiveNormal;
layout(binding = 1) uniform sampler2D ps_RefractiveDepth;
layout(binding = 2) uniform sampler2D ps_BackgroundDepth;

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_PhotonLocation;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
vec2 GetUVFromVSPosition(in vec4 _VSPosition)
{
    vec4 SSPosition = ps_LightProjectionMatrix * _VSPosition;

    return 0.5f * (SSPosition.xy / SSPosition.w) + 0.5f;
}

// -----------------------------------------------------------------------------

vec4 GetRefraction(in vec3 _IncidentRay, in vec3 _VSNormal, in float _RefractionIndex, in float _RefractionIndexSqr )
{
    float IdotN = dot(-_IncidentRay, _VSNormal);

    float CosineSqr = 1.0f - _RefractionIndexSqr * (1.0f - IdotN * IdotN);

    return CosineSqr <= 0.0f ? 
        vec4( reflect( _IncidentRay, _VSNormal ).xyz, -1.0f ) : 
        vec4( normalize( _RefractionIndex * _IncidentRay + (_RefractionIndex * IdotN - sqrt( CosineSqr )) * _VSNormal ).xyz, 1.0f); 
}

// -----------------------------------------------------------------------------

void main(void)
{
    vec3 WSNormal = in_Normal;
    vec3 VSNormal = (ps_LightViewMatrix * vec4(WSNormal, 0.0f)).xyz;

    vec3 SSPosition;

    SSPosition.xy = gl_FragCoord.xy / vec2(CAUSTIC_MAP_RESOLUTION);
    SSPosition.z  = gl_FragCoord.z;

    vec3 WSPosition = in_WSPosition;
    vec3 VSPosition = (ps_LightViewMatrix * vec4(WSPosition, 1.0f)).xyz;
 
    // -----------------------------------------------------------------------------
    // Stuff that we know from the beginning
    // -----------------------------------------------------------------------------
    vec3 VSNormalSurface1 = normalize(VSNormal);
    vec3 ViewDirection    = normalize(VSPosition);

    // -----------------------------------------------------------------------------
    // Find the distance to front & back surface, first as normalized [0..1] 
    // values, than unprojected
    // -----------------------------------------------------------------------------
    vec2 DepthOfBackAndObject = vec2(texture(ps_RefractiveDepth, SSPosition.xy).x, SSPosition.z);

    DepthOfBackAndObject = ps_DepthLinearization.x / (DepthOfBackAndObject * ps_DepthLinearization.y - ps_DepthLinearization.z );

    float Distance = DepthOfBackAndObject.y - DepthOfBackAndObject.x;
    
    // -----------------------------------------------------------------------------
    // Find the refraction direction of first surface
    // -----------------------------------------------------------------------------
    vec3 RefractionSurface1 = GetRefraction(ViewDirection, VSNormalSurface1, ps_RefractionIndices.x, ps_RefractionIndices.y).xyz; 

    // -----------------------------------------------------------------------------
    // Compute approximate exitant location & surface normal
    // -----------------------------------------------------------------------------
    vec4 VSExitantLocation = vec4(RefractionSurface1 * Distance + VSPosition.xyz, 1.0f);

    vec3 WSNormalSurf2 = texture(ps_RefractiveNormal, GetUVFromVSPosition(VSExitantLocation)).xyz;

    vec3 VSNormalSurface2 = (ps_LightViewMatrix * vec4(WSNormalSurf2, 0.0f)).xyz;

    float NdotN = dot(VSNormalSurface2.xyz, VSNormalSurface2.xyz);

    VSNormalSurface2 = normalize( VSNormalSurface2 );

    // -----------------------------------------------------------------------------
    // What happens if we lie in a black-texel? Means no normal! Conceptually,
    // this means we pass thru "side" of object. Use norm perpindicular to view
    // -----------------------------------------------------------------------------
    if (NdotN == 0.0f) VSNormalSurface2 = normalize(vec3(RefractionSurface1.x, RefractionSurface1.y, 0.0f));

    // -----------------------------------------------------------------------------
    // Refract at the second surface
    // -----------------------------------------------------------------------------
    vec4 RefractionSurface2 = GetRefraction(RefractionSurface1, -VSNormalSurface2, ps_RefractionIndices.z, ps_RefractionIndices.w );

    // -----------------------------------------------------------------------------
    // If we have an total internal reflection, we discard the pixel
    // -----------------------------------------------------------------------------
    if (RefractionSurface2.w < 0.0f && DISCARD_TIR) discard;
    
    // -----------------------------------------------------------------------------
    // Scale the vector so that it's got a unit-length z-component
    // -----------------------------------------------------------------------------
    vec4 ScaledRefractionSurface2 = vec4(RefractionSurface2.xyz, 0.0f) / -RefractionSurface2.z;

    // -----------------------------------------------------------------------------
    // Compute the texture locations of ctrPlusT2 and refractToNear.
    // -----------------------------------------------------------------------------
    float MinimalDistance = 1000.0f;
    float DeltaDistance   = 1000.0f;

    for (float Index = 0.0f; Index < 2.0f; Index += 1.0f)
    {
        float BackgroundDepth      = texture(ps_BackgroundDepth, GetUVFromVSPosition(VSExitantLocation + ScaledRefractionSurface2 * Index)).x;
        float DistanceToBackground = -(ps_DepthLinearization.x / (BackgroundDepth * ps_DepthLinearization.y - ps_DepthLinearization.w)) + VSExitantLocation.z;

        if (abs(DistanceToBackground - Index) < DeltaDistance )
        {
            DeltaDistance   = abs(DistanceToBackground - Index);
            MinimalDistance = Index;
        }
    }
    
    float DistanceToBackground = MinimalDistance;

    for (float Index = 0.0f; Index < 10.0f; Index += 1.0f)
    {
        float BackgroundDepth = texture(ps_BackgroundDepth, GetUVFromVSPosition(VSExitantLocation + DistanceToBackground * ScaledRefractionSurface2)).x;

        DistanceToBackground = -(ps_DepthLinearization.x / (BackgroundDepth * ps_DepthLinearization.y - ps_DepthLinearization.w)) + VSExitantLocation.z;
    }

    out_PhotonLocation.xyz = VSExitantLocation.xyz + DistanceToBackground * ScaledRefractionSurface2.xyz;
    out_PhotonLocation.w   = RefractionSurface2.w;
}

#endif // __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__