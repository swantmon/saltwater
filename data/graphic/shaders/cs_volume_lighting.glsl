#ifndef __INCLUDE_CS_VOLUME_LIGHTING_GLSL__
#define __INCLUDE_CS_VOLUME_LIGHTING_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -------------------------------------------------------------------------------------
// Input from engine
// -------------------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
{
    mat4   cs_LightViewProjection;
    vec4   cs_LightDirection;
    vec4   cs_LightColor;
    float  cs_SunAngularRadius;
    uint   cs_ExposureHistoryIndex;
};

layout(row_major, std140, binding = 2) uniform UB2
{
    vec4 cs_WindDirection;
    vec4 cs_FogColor;
    float cs_FrustumDepthInMeter;
    float cs_ShadowIntensity;
    float cs_VolumetricFogScatteringCoefficient;
    float cs_VolumetricFogAbsorptionCoefficient;
};

layout(std430, binding = 0) readonly buffer BB0
{
    float ps_ExposureHistory[8];
};

layout (binding = 0, rgba32f) writeonly uniform image3D cs_OutputImage;

layout (binding = 1, rgba32f) readonly uniform image2D cs_PermutationImage;
layout (binding = 2, rgba32f) readonly uniform image2D cs_PermutationGradientImage;
layout (binding = 3, r32f)    readonly uniform image2D cs_ESMTexture;

// -------------------------------------------------------------------------------------
// Simon Green. 
// “Implementing Improved Perlin Noise.” 
// In GPU Gems 2, edited by Matt Farr, pp. 409–416, Addison-Wesley Professional, 2005.
// -------------------------------------------------------------------------------------
vec3 GetFade(in vec3 t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); // new curve
//  return t * t * (3 - 2 * t); // old curve
}

vec4 SamplePermutation(in vec2 _UV)
{
    return imageLoad(cs_PermutationImage, ivec2(_UV * vec2(256.0f)));
}

float SampleGradientPermutation(in float _U, in vec3 _Point)
{
    return dot(imageLoad(cs_PermutationGradientImage, ivec2(_U * 256, 0)).xyz, _Point);
}

float ImprovedPerlinNoise3D(in vec3 _Seed)
{
    // -------------------------------------------------------------------------------------
    // 1. FIND UNIT CUBE THAT CONTAINS POINT
    // 2. FIND RELATIVE X,Y,Z OF POINT IN CUBE.
    // 3. COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
    // -------------------------------------------------------------------------------------
    vec3 UV = mod(floor(_Seed), vec3(256.0f));
    _Seed -= floor(_Seed);

    vec3 Fade = GetFade(_Seed);

    UV = UV / 256.0f;

    const float One = 1.0f / 256.0f;
    
    // -------------------------------------------------------------------------------------
    // HASH COORDINATES OF THE 8 CUBE CORNERS
    // -------------------------------------------------------------------------------------
    vec4 Hash = SamplePermutation(UV.xy) + UV.z;
 
    // -------------------------------------------------------------------------------------
    // AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
    // -------------------------------------------------------------------------------------
    return mix( mix( mix( SampleGradientPermutation(Hash.x, _Seed ),  
                          SampleGradientPermutation(Hash.z, _Seed + vec3(-1.0f,  0.0f, 0.0f) ), Fade.x),
                     mix( SampleGradientPermutation(Hash.y, _Seed + vec3( 0.0f, -1.0f, 0.0f) ),
                          SampleGradientPermutation(Hash.w, _Seed + vec3(-1.0f, -1.0f, 0.0f) ), Fade.x), Fade.y),
                           
                mix( mix( SampleGradientPermutation(Hash.x + One, _Seed + vec3( 0.0f,  0.0f, -1.0f) ),
                          SampleGradientPermutation(Hash.z + One, _Seed + vec3(-1.0f,  0.0f, -1.0f) ), Fade.x),
                     mix( SampleGradientPermutation(Hash.y + One, _Seed + vec3( 0.0f, -1.0f, -1.0f) ),
                          SampleGradientPermutation(Hash.w + One, _Seed + vec3(-1.0f, -1.0f, -1.0f) ), Fade.x), Fade.y), Fade.z);
}

layout(local_size_x = 16, local_size_y = 10, local_size_z = 8) in;
void main()
{
    uint X;
    uint Y;
    uint Z;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;
    Z = gl_GlobalInvocationID.z;
    
    // -------------------------------------------------------------------------------------
    // World position from frustum
    // -------------------------------------------------------------------------------------
    vec2  TexCoord    = vec2(X / 160.0f, Y / 90.0f);
    float LinearDepth = max(Z * cs_FrustumDepthInMeter / 128.0f, 0.00001f);
    
    float SSDepth = ConvertToHyperbolicDepth(LinearDepth, ps_ViewToScreen);
    
    SSDepth = SSDepth * 0.5f + 0.5f;
    
    vec3 VSPosition = GetViewSpacePositionFromDepth(SSDepth, TexCoord, ps_ScreenToView);
    vec3 WSPosition = (ps_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;
    
    // -------------------------------------------------------------------------------------
    // Mie and Rayleigh scattering
    // -------------------------------------------------------------------------------------
    vec3 WSLightDirection = -cs_LightDirection.xyz;
    vec3 WSViewDirection  = normalize(ps_ViewPosition.xyz - WSPosition);
    
    float LdotV  = dot(WSLightDirection, WSViewDirection) / length(WSViewDirection);
    float LdotV2 = LdotV * LdotV;
    
    float Symmetry = -0.95f;
    float Symmetry2 = Symmetry * Symmetry;

    float RayleighPhase = 0.75f * (1.0f + LdotV2);
    float MiePhase      = 1.5f * ((1.0f - Symmetry2) / (2.0f + Symmetry2)) * (1.0f + LdotV2) / pow(abs(1.0f + Symmetry2 - 2.0f * Symmetry * LdotV), 1.5f);
    
    // -----------------------------------------------------------------------------
    // Thickness of slice
    // -----------------------------------------------------------------------------
    float Thickness = 1.0f / cs_FrustumDepthInMeter;
    
    // -----------------------------------------------------------------------------
    // Density of dust
    // -----------------------------------------------------------------------------
    vec3 Seed;
    
    Seed  = WSPosition.xyz;
    Seed += cs_WindDirection.xyz;
    Seed *= Thickness;
    
    float Density = abs(ImprovedPerlinNoise3D(Seed)) * (RayleighPhase + MiePhase);
    
    // -----------------------------------------------------------------------------
    // Scattering
    // -----------------------------------------------------------------------------
    float Scattering = cs_VolumetricFogScatteringCoefficient * Density * Thickness;
    
    // -----------------------------------------------------------------------------
    // Absorption
    // -----------------------------------------------------------------------------
    float Absorption = cs_VolumetricFogAbsorptionCoefficient * Density * Thickness;
    
    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[cs_ExposureHistoryIndex];
    
    // -----------------------------------------------------------------------------
    // Sun lighting
    // -----------------------------------------------------------------------------
    vec4 LSPosition = cs_LightViewProjection * vec4(WSPosition, 1.0f);
   
    LSPosition.xyz /= LSPosition.w;
    
    vec3 ShadowCoord = LSPosition.xyz * 0.5f + 0.5f;

    float DepthValue = imageLoad(cs_ESMTexture, ivec2(vec2(ShadowCoord.x, ShadowCoord.y) * vec2(256.0f, 256.0f))).r;

    float Shadow = 1.0f;
    
    if (ShadowCoord.z > DepthValue)
    {
        Shadow = DepthValue / cs_ShadowIntensity;
    }
    
    vec3 Lighting = vec3(0.0f);
    
    Lighting += AverageExposure * cs_LightColor.xyz * Shadow;

    // -----------------------------------------------------------------------------
    // Ambient lighting 
    // TODO: Get ambient light from skybox
    // -----------------------------------------------------------------------------
    Lighting += vec3(0.0f);

    // -----------------------------------------------------------------------------
    // Other lights (spot, point, ...)
    // TODO: Iterate over maximum of four lights
    // -----------------------------------------------------------------------------
    
    // -----------------------------------------------------------------------------
    // Fog color
    // -----------------------------------------------------------------------------
    Lighting *= cs_FogColor.rgb * cs_FogColor.a;
    
   
    imageStore(cs_OutputImage, ivec3(X, Y, Z), vec4(Lighting * Scattering, Scattering + Absorption));
}

#endif // __INCLUDE_CS_VOLUME_LIGHTING_GLSL__