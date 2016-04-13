
// -----------------------------------------------------------------------------
// Code in dependence of:
// 1. Efficient GPU Screen-Space Ray Tracing, Mcguire M., 2014
// 2. Unreal Engine Source Code (UE 4.1)
//  - ScreenSpaceReflections.usf
//  - ScreenSpaceRayCast.usf
// -----------------------------------------------------------------------------

#ifndef __INCLUDE_FS_SSR_GLSL__
#define __INCLUDE_FS_SSR_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

#include "fs_global.glsl"

// -----------------------------------------------------------------------------
// Defines / Settings
// -----------------------------------------------------------------------------
#define SSR_CONE_QUALITY

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform USSRProperties
{
    vec4 ps_ConstantBufferData0;
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_Depth;
layout(binding = 4) uniform sampler2D ps_LightAccumulationBuffer;
layout(binding = 5) uniform sampler2D ps_BRDF;

// -----------------------------------------------------------------------------
// Easy access
// MaxRoughness         = Clamp(ScreenSpaceReflectionMaxRoughness, 0.01f, 1.0f);
// ps_SSRRougnessMaskScale = -2.0f / MaxRoughness;
// -----------------------------------------------------------------------------
#define ps_SSRIntensity         ps_ConstantBufferData0.x
#define ps_SSRRougnessMaskScale ps_ConstantBufferData0.y
#define ps_PreviousFrame        ps_ConstantBufferData0.z

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec4 SampleLightFromTexture(in sampler2D _Texture, in vec3 _HitUVz, in float _Level)
{
    vec4  OutColor;
    float DistanceFromCenter;

    if (ps_PreviousFrame == 1.0f)
    {
        vec4 HitClip            = vec4((_HitUVz.xy - ps_ScreenPositionScaleBias.zw) / ps_ScreenPositionScaleBias.xy, _HitUVz.z, 1.0f );
        vec4 HitTranslatedWorld = ps_ViewToWorld * ps_ScreenToView * HitClip;
            
        vec4 PreviousTranslatedWorld = vec4(HitTranslatedWorld.xyz + HitTranslatedWorld.w * (ps_PreviousViewPosition.xyz - ps_ViewPosition.xyz), HitTranslatedWorld.w);
        vec4 PreviousClip            = ps_PreviousViewToScreen * ps_PreviousWorldToView * PreviousTranslatedWorld;
        vec2 PreviousScreen          = PreviousClip.xy / PreviousClip.w;
        vec2 PreviousUV              = PreviousScreen.xy * ps_ScreenPositionScaleBias.xy + ps_ScreenPositionScaleBias.zw;
        
        OutColor.rgb = textureLod( _Texture, PreviousUV.xy, _Level).rgb;
        OutColor.a = 1;

        DistanceFromCenter = distance(PreviousUV.xy, vec2(0.5f)) * 2.0f;
    }
    else
    {
        OutColor.rgb = textureLod( _Texture, _HitUVz.xy, _Level).rgb;
        OutColor.a = 1;

        DistanceFromCenter = distance(_HitUVz.xy, vec2(0.5f)) * 2.0f;
    }

    // -----------------------------------------------------------------------------
    // Off screen masking
    // -----------------------------------------------------------------------------
    float FADE_START = 0.0f;
    float FADE_END   = 1.0f;
    
    OutColor *= (1.0f - clamp((DistanceFromCenter - FADE_START) / (FADE_END - FADE_START), 0.0f, 1.0f));

    // -----------------------------------------------------------------------------
    // Transform NaNs to black, transform negative colors to black.
    // -----------------------------------------------------------------------------
    OutColor.rgb = -min(-OutColor.rgb, 0.0);

    return OutColor;
}

// -----------------------------------------------------------------------------

vec4 SampleDepthFromTexture(in sampler2D _Texture, in vec4 _SampleUV0, in vec4 _SampleUV1)
{
    vec4 SampleDepth;

    SampleDepth.x = textureLod( _Texture, _SampleUV0.xy, 0 ).r;
    SampleDepth.y = textureLod( _Texture, _SampleUV0.zw, 0 ).r;
    SampleDepth.z = textureLod( _Texture, _SampleUV1.xy, 0 ).r;
    SampleDepth.w = textureLod( _Texture, _SampleUV1.zw, 0 ).r;

    return SampleDepth;
}

// -----------------------------------------------------------------------------
// This function shoots a ray in screen space along a given direction in 
// world-space. Inside this function a line is build in screen-space along the
// direction. After this four samples are sampled form texture and checked which
// one is the closest one with a hit.
// -----------------------------------------------------------------------------
void RayCast(
    in sampler2D _Texture,
    in vec3 _RayOriginTranslatedWorld, in vec3 _RayDirection,
    in float _Roughness, in float _ConeAngleWorld, in float _SceneDepth,
    in int _NumSteps, in float _StepOffset,
    out vec4 _OutHitUVzTime, out float _OutHCBLevel
)
{
    // -----------------------------------------------------------------------------
    // Transform ray from world to view and from view to screen/clip
    // -----------------------------------------------------------------------------
    const vec4 RayStartV = ps_WorldToView * vec4(_RayOriginTranslatedWorld, 1.0f);
    const vec4 RayDirV   = ps_WorldToView * vec4(_RayDirection * _SceneDepth, 0.0f);
    const vec4 RayEndV   = RayStartV + RayDirV;

    const float RayEndRadiusV = length(RayDirV.xyz) * tan(_ConeAngleWorld * 0.4f);
    const vec2  RayEndBorderV = RayEndV.xy + RayEndRadiusV / sqrt(2.0f);

    const vec4 RayStartClip         = ps_ViewToScreen * RayStartV;
    const vec4 RayEndClip           = ps_ViewToScreen * RayEndV;
    const vec4 TempRayEndBorderClip = ps_ViewToScreen * vec4(RayEndBorderV, RayEndV.zw);
    const vec2 RayEndBorderClip     = TempRayEndBorderClip.xy;

    const vec3 RayStartScreen     = RayStartClip.xyz / RayStartClip.w;
    const vec3 RayEndScreen       = RayEndClip.xyz / RayEndClip.w;
    const vec2 RayEndBorderScreen = RayEndBorderClip.xy / RayEndClip.w;
    
    vec3 RayStepScreen       = (RayEndScreen - RayStartScreen);
    vec2 RayStepRadiusScreen = (RayEndBorderScreen - RayEndScreen.xy);

    // -----------------------------------------------------------------------------
    // Rescale step
    // Computes the scale down factor for RayStepScreen required to fit on 
    // the X and Y axis in order to clip it in the viewport
    // -----------------------------------------------------------------------------
    const float RayStepScreenInvFactor = 0.5f * length( RayStepScreen.xy );
    const vec2  AbsRayStepScreen       = abs(RayStepScreen.xy);
    const vec2  S                      = (AbsRayStepScreen - max(abs(RayStepScreen.xy + RayStartScreen.xy * RayStepScreenInvFactor) - RayStepScreenInvFactor, 0.0f)) / AbsRayStepScreen;
    const float RayStepFactor          = min(S.x, S.y) / RayStepScreenInvFactor;

    RayStepScreen       *= RayStepFactor;
    RayStepRadiusScreen *= RayStepFactor;

    // -----------------------------------------------------------------------------
    // Transform from screen space to UV coordinates
    // -----------------------------------------------------------------------------
    const vec3 RayStartUVz     = vec3((RayStartScreen.xy * ps_ScreenPositionScaleBias.xy + ps_ScreenPositionScaleBias.zw), RayStartScreen.z);
    const vec3 RayStepUVz      = vec3(RayStepScreen.xy   * ps_ScreenPositionScaleBias.xy, RayStepScreen.z);
    const vec2 RayStepRadiusUV = RayStepRadiusScreen.xy  * ps_ScreenPositionScaleBias.xy;
    
    // -----------------------------------------------------------------------------
    // Step width
    // -----------------------------------------------------------------------------
    const float RayStepRadiusFactor = length(RayStepRadiusUV * ps_InvertedScreensizeAndScreensize.zw);
    const float Step = 1.0f / (_NumSteps + 1);

    // -----------------------------------------------------------------------------
    // *2 to get less morie pattern in extreme cases, larger values make object 
    // appear not grounded in reflections
    // -----------------------------------------------------------------------------
    const float CompareTolerance = abs(RayStepUVz.z) * Step * 2.0f;

    // -----------------------------------------------------------------------------
    // Avoid bugs with early returns inside of loops on certain platform compilers.
    // -----------------------------------------------------------------------------
    vec4 Result = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    float LastDiff = 0.0f;

    vec4 SampleTime = (_StepOffset + vec4(1.0f, 2.0f, 3.0f, 4.0f)) * Step;

    // -----------------------------------------------------------------------------
    // Go along the ray with four step at once (called section)
    // -----------------------------------------------------------------------------
    #pragma unroll
    for( int IndexOfSection = 0; IndexOfSection < _NumSteps; IndexOfSection += 4 )
    {
        // -----------------------------------------------------------------------------
        // Vectorized to group fetches
        // -----------------------------------------------------------------------------
        vec4 SampleUV0 = RayStartUVz.xyxy + RayStepUVz.xyxy * SampleTime.xxyy;
        vec4 SampleUV1 = RayStartUVz.xyxy + RayStepUVz.xyxy * SampleTime.zzww;
        vec4 SampleZ   = RayStartUVz.zzzz + RayStepUVz.zzzz * SampleTime;
        
        // -----------------------------------------------------------------------------
        // Use lower res for farther samples
        // Note: Lower resolution samples aren't used by now
        // -----------------------------------------------------------------------------
        vec4 SampleDepth = SampleDepthFromTexture( _Texture, SampleUV0, SampleUV1 );

        // -----------------------------------------------------------------------------
        // Check hit on latest four samples
        // -----------------------------------------------------------------------------
        vec4 DepthDiff1 = SampleZ - SampleDepth;
        bvec4 Hit;

        Hit.x = abs( -DepthDiff1.x - CompareTolerance ) < CompareTolerance;
        Hit.y = abs( -DepthDiff1.y - CompareTolerance ) < CompareTolerance;
        Hit.z = abs( -DepthDiff1.z - CompareTolerance ) < CompareTolerance;
        Hit.w = abs( -DepthDiff1.w - CompareTolerance ) < CompareTolerance;

        // -----------------------------------------------------------------------------
        // If we have a hit on the section
        // -----------------------------------------------------------------------------
        if( any( Hit ) )
        {
            // -----------------------------------------------------------------------------
            // Take closest hit in section
            // -----------------------------------------------------------------------------
            vec4 DepthDiff0 = vec4( LastDiff, DepthDiff1.xyz );
            vec4 TimeLerp = clamp( DepthDiff0 / (DepthDiff0 - DepthDiff1) , 0.0f, 1.0f);
            vec4 IntersectTime = SampleTime + (TimeLerp - 1.0f) / (_NumSteps + 1);

            vec4 HitTime;

            HitTime.x = Hit.x ? IntersectTime.x : 1.0f;
            HitTime.y = Hit.y ? IntersectTime.y : 1.0f;
            HitTime.z = Hit.z ? IntersectTime.z : 1.0f;
            HitTime.w = Hit.w ? IntersectTime.w : 1.0f;

            HitTime.xy = min( HitTime.xy, HitTime.zw );
            float MinHitTime = min( HitTime.x, HitTime.y );
            
            vec3 HitUVz = RayStartUVz + RayStepUVz * MinHitTime;

            Result = vec4( HitUVz, MinHitTime );
            break;
        }

        // -----------------------------------------------------------------------------
        // Compute next section
        // -----------------------------------------------------------------------------
        LastDiff = DepthDiff1.w;

        SampleTime += 4.0f / (float(_NumSteps + 1));
    }

    // -----------------------------------------------------------------------------
    // Return result
    // -----------------------------------------------------------------------------
    _OutHitUVzTime = Result;
    _OutHCBLevel   = log2(_OutHitUVzTime.w * RayStepRadiusFactor) + 1.0f;
}

// -----------------------------------------------------------------------------

float GetRoughnessFade(in float _Roughness)
{
    // -----------------------------------------------------------------------------
    // Mask SSR to reduce noise and for better performance, roughness of 0 should 
    // have SSR, at MaxRoughness we fade to 0
    // -----------------------------------------------------------------------------
    return min(_Roughness * ps_SSRRougnessMaskScale + 2.0f, 1.0f);
}

// -----------------------------------------------------------------------------

uint ReverseUIntBits(in uint _Bits)
{
    //_Bits = ( _Bits << 16) | ( _Bits >> 16);
    //_Bits = ( (_Bits & 0x00ff00ff) << 8 ) | ( (_Bits & 0xff00ff00) >> 8 );
    //_Bits = ( (_Bits & 0x0f0f0f0f) << 4 ) | ( (_Bits & 0xf0f0f0f0) >> 4 );
    _Bits = ( (_Bits & 0x33333333) << 2 ) | ( (_Bits & 0xcccccccc) >> 2 );
    _Bits = ( (_Bits & 0x55555555) << 1 ) | ( (_Bits & 0xaaaaaaaa) >> 1 );
    return _Bits;
}

// -----------------------------------------------------------------------------

uint MortonCode(in uint _Code)
{
    //_Code = (_Code ^ (_Code <<  8)) & 0x00ff00ff;
    //_Code = (_Code ^ (_Code <<  4)) & 0x0f0f0f0f;
    _Code = (_Code ^ (_Code <<  2)) & 0x33333333;
    _Code = (_Code ^ (_Code <<  1)) & 0x55555555;
    return _Code;
}

// -----------------------------------------------------------------------------

uint ReverseBits32(in uint Bits)
{
    Bits = ( Bits << 16) | ( Bits >> 16);
    Bits = ( (Bits & 0x00ff00ff) << 8 ) | ( (Bits & 0xff00ff00) >> 8 );
    Bits = ( (Bits & 0x0f0f0f0f) << 4 ) | ( (Bits & 0xf0f0f0f0) >> 4 );
    Bits = ( (Bits & 0x33333333) << 2 ) | ( (Bits & 0xcccccccc) >> 2 );
    Bits = ( (Bits & 0x55555555) << 1 ) | ( (Bits & 0xaaaaaaaa) >> 1 );
    return Bits;
}

// -----------------------------------------------------------------------------

#ifdef SSR_CONE_QUALITY

    float PseudoRandom(in vec2 _Coord)
    {
        vec2 Random = fract(_Coord / 128.0f) * 128.0f + vec2(-64.340622f, -72.465622f);
        
        // -----------------------------------------------------------------------------
        // found by experimentation
        // -----------------------------------------------------------------------------
        return fract(dot(Random.xyx * Random.xyy, vec3(20.390625f, 60.703125f, 2.4281209f)));
    }

    // -----------------------------------------------------------------------------

    vec3 TangentToWorld( vec3 Vec, vec3 TangentZ )
    {
        vec3 UpVector = abs(TangentZ.y) < 0.999 ? vec3(0,1,0) : vec3(1,0,0);
        vec3 TangentX = normalize( cross( UpVector, TangentZ ) );
        vec3 TangentY = cross( TangentZ, TangentX );
        return TangentX * Vec.x + TangentY * Vec.y + TangentZ * Vec.z;
    }

    // -----------------------------------------------------------------------------

    float ClampedPow(float X, float Y)
    {
        return pow(max(abs(X),0.000001f),Y);
    }

    // -----------------------------------------------------------------------------

    vec2 ClampedPow(vec2 X,vec2 Y)
    {
        return pow(max(abs(X),vec2(0.000001f,0.000001f)),Y);
    }

    // -----------------------------------------------------------------------------

    vec3 ClampedPow(vec3 X,vec3 Y)
    {
        return pow(max(abs(X),vec3(0.000001f,0.000001f,0.000001f)),Y);
    }  

    // -----------------------------------------------------------------------------

    vec4 ClampedPow(vec4 X,vec4 Y)
    {
        return pow(max(abs(X),vec4(0.000001f,0.000001f,0.000001f,0.000001f)),Y);
    } 

    // -----------------------------------------------------------------------------

    vec4 ImportanceSampleBlinn( vec2 E, float Roughness )
    {
        float m = Roughness * Roughness;
        float n = 2 / (m*m) - 2;

        float Phi = 2 * PI * E.x;
        float CosTheta = ClampedPow( E.y, 1 / (n + 1) );
        float SinTheta = sqrt( 1 - CosTheta * CosTheta );

        vec3 H;
        H.x = SinTheta * cos( Phi );
        H.y = SinTheta * sin( Phi );
        H.z = CosTheta;

        float D = (n+2)/ (2*PI) * ClampedPow( CosTheta, n );
        float PDF = D * CosTheta;

        return vec4( H, PDF );
    }
#endif

// -----------------------------------------------------------------------------

void main(void)
{
    float RoughnessFade;
    vec2  PixelPos;
    vec2  ScreenPos;

    // -----------------------------------------------------------------------------
    // Calculate screen and pixel position
    // Info: screen position in [-1, 1] screen space
    // Info: pixel position in [0,0 -> width-1, height-1]
    // -----------------------------------------------------------------------------
    ScreenPos.xy = (in_UV - ps_ScreenPositionScaleBias.zw) / ps_ScreenPositionScaleBias.xy;
    PixelPos.xy  = ScreenPos * ps_InvertedScreensizeAndScreensize.zw + (ps_InvertedScreensizeAndScreensize.zw / vec2(2.0f)) + 0.5f;

    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0, in_UV);
    vec4  GBuffer1 = texture(ps_GBuffer1, in_UV);
    vec4  GBuffer2 = texture(ps_GBuffer2, in_UV);
    float VSDepth  = texture(ps_Depth   , in_UV).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_UV, ps_ScreenToView);
    
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
    // Roughness Fade
    // -----------------------------------------------------------------------------
    RoughnessFade = GetRoughnessFade(Data.m_Roughness);

    if( RoughnessFade <= 0.0 || VSDepth == 1.0f)
    {
        return;
    }


#ifdef DEBUG


    // -----------------------------------------------------------------------------
    // Just a debug output if needed:
    // activate with #define DEBUG 1
    // -----------------------------------------------------------------------------

    float PatternMask = mod((PixelPos.x / 2.0f + PixelPos.y / 2.0f), 2.0f) * 0.7f;

    out_Output = mix(vec4(1, 0, 0, 1), vec4(1, 1 ,0, 1), PatternMask) * 0.3f;

    return;


#endif


    // -----------------------------------------------------------------------------
    // Vectors
    // -----------------------------------------------------------------------------
    const vec3 WSViewDirection = normalize(ps_ViewPosition.xyz - Data.m_WSPosition);
    
    // -----------------------------------------------------------------------------
    // Frame
    // -----------------------------------------------------------------------------
    uint FrameRandom = 0;

    const uint RandomizeOverNFrames = 8;
    FrameRandom = (uint(0) % RandomizeOverNFrames) * 1551;

    const int NumSteps = 64;
    const int NumRays  = 1;


#ifdef SSR_CONE_QUALITY


    PixelPos.xy  = ScreenPos * ps_InvertedScreensizeAndScreensize.zw + ps_InvertedScreensizeAndScreensize.zw / vec2(2.0f);
    const vec2 E = vec2(PseudoRandom(PixelPos + ivec2(FrameRandom, 0)), PseudoRandom(PixelPos + ivec2(0, FrameRandom)));

    const float StepOffset = PseudoRandom(PixelPos + FrameRandom);

    const vec4 HNAndPDF          = ImportanceSampleBlinn(E, Data.m_Roughness);
    const vec3 H                 = TangentToWorld(HNAndPDF.xyz, Data.m_WSNormal);
    const vec3 ViewMirrorUnitDir = 2.0f * dot( WSViewDirection, H ) * H - WSViewDirection;

    const float ReflectPDF       = HNAndPDF.w / (4.0f * clamp(dot(WSViewDirection, H), 0.0f, 1.0f));
    const float TotalSamples     = 8.0f;
    const float SolidAngleSample = 1.0 / (TotalSamples * ReflectPDF);
    const float ConeAngleWorld   = acos(1.0f - SolidAngleSample / (2.0f * PI));

    vec4 HitUVzTime;
    float HCBLevel;
    
    RayCast(
        ps_Depth,
        Data.m_WSPosition, ViewMirrorUnitDir, Data.m_Roughness, ConeAngleWorld, Data.m_VSDepth,
        NumSteps, StepOffset,
        HitUVzTime, HCBLevel
    );

    // -----------------------------------------------------------------------------
    // Is there a hit?
    // -----------------------------------------------------------------------------
    if( HitUVzTime.w < 1 )
    {
        vec3 LightColor = SampleLightFromTexture(ps_LightAccumulationBuffer, HitUVzTime.xyz, HCBLevel).rgb;

        float NdotV  = clamp( dot( Data.m_WSNormal, WSViewDirection), 0.0, 1.0f); 

        vec3 PreDFGF = textureLod(ps_BRDF, vec2(NdotV, Data.m_Roughness), 0).rgb;

        float F90 = clamp(50.0f * dot(Data.m_SpecularAlbedo, vec3(0.33f)), 0.0f, 1.0f);

        vec3 Specular = LightColor * (Data.m_SpecularAlbedo * PreDFGF.x + F90 * PreDFGF.y) * Data.m_AmbientOcclusion;

        out_Output = vec4(Specular, 1.0f - HitUVzTime.z);
    }


#else // SSR_CONE_QUALITY


    // -----------------------------------------------------------------------------
    // Sample set dithered over 4x4 pixels
    // -----------------------------------------------------------------------------
    uint Morton     = MortonCode(uint(PixelPos.x) & 3) | (MortonCode(uint(PixelPos.y) & 3) * 2);
    uint PixelIndex = ReverseUIntBits(Morton);

    // -----------------------------------------------------------------------------
    // Removed to shoot several rays (only one bounce supported)
    // -----------------------------------------------------------------------------
    uint  Offset     = (PixelIndex + ReverseUIntBits(FrameRandom)) & 15;
    float StepOffset = float(Offset) / 15.0f;

    StepOffset -= 0.5f;
    
    vec3 ViewMirrorUnitDir = reflect( -WSViewDirection, Data.m_WSNormal );
    
    vec4  HitUVzTime;
    float HCBLevel;

    RayCast(
        ps_Depth,
        Data.m_WSPosition, ViewMirrorUnitDir, Data.m_Roughness, 0.001f, Data.m_VSDepth,
        NumSteps, StepOffset,
        HitUVzTime, HCBLevel
    );

    // -----------------------------------------------------------------------------
    // Is there a hit?
    // -----------------------------------------------------------------------------
    if( HitUVzTime.w < 1 )
    {
        vec3 LightColor = SampleLightFromTexture(ps_LightAccumulationBuffer, HitUVzTime.xyz, HCBLevel).rgb;      

        vec3 Specular = LightColor * Data.m_SpecularAlbedo * Data.m_AmbientOcclusion;

        out_Output = vec4(Specular, 1.0f - HitUVzTime.z);
    }


#endif 

    out_Output *= ps_SSRIntensity;
    out_Output *= clamp(RoughnessFade, 0.0f, 1.0f);

}

#endif // __INCLUDE_FS_SSR_GLSL__