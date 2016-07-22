#ifndef __INCLUDE_CS_VOLUME_LIGHTING_GLSL__
#define __INCLUDE_CS_VOLUME_LIGHTING_GLSL__

#include "fs_global.glsl"

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

// -------------------------------------------------------------------------------------
// Input from engine
// -------------------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform USunLightProperties
{
    mat4   ps_LightViewProjection;
    vec4   ps_LightDirection;
    vec4   ps_LightColor;
    float  ps_SunAngularRadius;
    uint   ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) readonly buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};


layout (binding = 0, rgba32f) writeonly uniform image3D cs_OutputImage;

layout (binding = 1, rgba32f) readonly uniform image2D cs_PermutationImage;
layout (binding = 2, rgba32f) readonly uniform image2D cs_PermutationGradientImage;

layout (binding = 3, rgba8) readonly uniform image2D cs_GBuffer0;
layout (binding = 4, rgba8) readonly uniform image2D cs_GBuffer1;
layout (binding = 5, rgba8) readonly uniform image2D cs_GBuffer2;
layout (binding = 6) uniform sampler2D cs_DepthTexture;
layout (binding = 7, r32f) readonly uniform image2D cs_ESMTexture;

// -------------------------------------------------------------------------------------
// Simon Green. 
// “Implementing Improved Perlin Noise.” 
// In GPU Gems 2, edited by Matt Farr, pp. 409–416, Addison-Wesley Professional, 2005.
// -------------------------------------------------------------------------------------
vec3 GetFade(in vec3 t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); // new curve
//	return t * t * (3 - 2 * t); // old curve
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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    vec4 Pixel;
    vec4 Lighting;
    vec3 Seed;
    vec3 Wind;
    float PerlinNoise;
    uint X;
    uint Y;
    uint Z;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;
    Z = gl_GlobalInvocationID.z;

    // -------------------------------------------------------------------------------------
    // Getting wind and apply to seed
    // -------------------------------------------------------------------------------------
    Wind = vec3(ps_SunAngularRadius);

    Seed  = vec3(X / 160.0f, Y / 90.0f, Z / 128.0f);
    Seed += Wind;

    // -------------------------------------------------------------------------------------
    // Calculate Perlin Noise at this position in space
    // -------------------------------------------------------------------------------------
    PerlinNoise = ImprovedPerlinNoise3D(Seed);

    // -------------------------------------------------------------------------------------
    // Calculate Perlin Noise at this position in space
    // -------------------------------------------------------------------------------------
    Lighting = vec4(PerlinNoise * 10.0f);


    vec2 TexCoord = vec2(X / 160.0f, Y / 90.0f);

    vec4  GBuffer0 = imageLoad(cs_GBuffer0, ivec2(TexCoord * vec2(1280.0f, 720.0f)));
    vec4  GBuffer1 = imageLoad(cs_GBuffer1, ivec2(TexCoord * vec2(1280.0f, 720.0f)));
    vec4  GBuffer2 = imageLoad(cs_GBuffer2, ivec2(TexCoord * vec2(1280.0f, 720.0f)));

    float VSDepth  = texture(cs_DepthTexture, TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, TexCoord, ps_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (ps_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    UnpackGBuffer(GBuffer0, GBuffer1, GBuffer2, WSPosition.xyz, VSDepth, Data);

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];
    
    // -----------------------------------------------------------------------------
    // Compute lighting for sun light
    // -----------------------------------------------------------------------------
    vec3 WSLightDirection  = -ps_LightDirection.xyz;
    vec3 WSViewDirection   = normalize(ps_ViewPosition.xyz - Data.m_WSPosition);
    
    float NdotV = dot(Data.m_WSNormal, WSViewDirection);
    
    vec3 ViewMirrorUnitDir = 2.0f * NdotV * Data.m_WSNormal - WSViewDirection;
    
    // -----------------------------------------------------------------------------
    // Compute sun data
    // -----------------------------------------------------------------------------
    float r = sin(ps_SunAngularRadius);
    float d = cos(ps_SunAngularRadius);

    float DdotR = dot(WSLightDirection, ViewMirrorUnitDir);
    vec3  S     = ViewMirrorUnitDir - DdotR * WSLightDirection;
    vec3  L     = DdotR < d ? normalize(d * WSLightDirection + normalize(S) * r) : ViewMirrorUnitDir;
    

    // -----------------------------------------------------------------------------
    // Shadow
    // -----------------------------------------------------------------------------
    vec4  LSPosition;
    vec3  ShadowCoord;
    float DepthValue;
    float Shadow;

    // -----------------------------------------------------------------------------
    // Set world space coord into light projection by multiply with light
    // view and projection matrix;
    // -----------------------------------------------------------------------------
    LSPosition = ps_LightViewProjection * vec4(Data.m_WSPosition, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Divide xyz by w to get the position in light view's clip space.
    // -----------------------------------------------------------------------------
    LSPosition.xyz /= LSPosition.w;
    
    // -----------------------------------------------------------------------------
    // Get uv texcoords for this position
    // -----------------------------------------------------------------------------
    ShadowCoord = LSPosition.xyz * 0.5f + 0.5f;
    
    // -----------------------------------------------------------------------------
    // Get final depth at this texcoord and compare it with the real
    // position z value (do a manual depth test)
    // -----------------------------------------------------------------------------
    DepthValue = imageLoad(cs_ESMTexture, ivec2(vec2(ShadowCoord.x, ShadowCoord.y) * vec2(256.0f, 256.0f))).r;

    Shadow = 1.0f;
    
    if (ShadowCoord.z + 0.001f > DepthValue)
    {
        Shadow = 0.0f;
    }

    // -----------------------------------------------------------------------------
    // Apply light luminance
    // -----------------------------------------------------------------------------
    vec3 Luminance = BRDF(L, WSViewDirection, Data.m_WSNormal, Data) * clamp(dot(Data.m_WSNormal, L), 0.0f, 1.0f) * ps_LightColor.xyz * Data.m_AmbientOcclusion * Shadow;

    //Luminance *= AverageExposure;
    
    Lighting *= vec4(Luminance, 1.0f);

    imageStore(cs_OutputImage, ivec3(X, Y, Z), Lighting);
}

#endif // __INCLUDE_CS_VOLUME_LIGHTING_GLSL__