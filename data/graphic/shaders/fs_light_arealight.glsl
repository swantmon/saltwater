
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__

#include "common.glsl"
#include "common_global.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_raycast.glsl"

// -----------------------------------------------------------------------------
// Input from system
// -----------------------------------------------------------------------------
in vec4 gl_FragCoord;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
const float LUT_SIZE  = 64.0f;
const float LUT_SCALE = (LUT_SIZE - 1.0f) / LUT_SIZE;
const float LUT_BIAS  = 0.5f / LUT_SIZE;

layout(row_major, std140, binding = 1) uniform UB1
{
    vec4  dcolor;
    vec4  scolor;
    float intensity;
    float width;
    float height;
    float rotz;
    float padding;
    bool  twoSided;
    uint  ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_DepthTexture;
layout(binding = 4) uniform sampler2D ps_LTCMaterial;
layout(binding = 5) uniform sampler2D ps_LTCMag;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output to light accumulation target
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Linearly Transformed Cosines
// -----------------------------------------------------------------------------
float IntegrateEdge(in vec3 _V1, in vec3 _V2)
{
    float CosTheta = dot(_V1, _V2);
    float Theta    = acos(CosTheta);    
    float Result   = cross(_V1, _V2).z * ((Theta > 0.001f) ? Theta / sin(Theta) : 1.0f);

    return Result;
}

// -----------------------------------------------------------------------------

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // -----------------------------------------------------------------------------
    // detect clipping config
    // -----------------------------------------------------------------------------
    int config = 0;

    if (L[0].z > 0.0) config += 1;
    if (L[1].z > 0.0) config += 2;
    if (L[2].z > 0.0) config += 4;
    if (L[3].z > 0.0) config += 8;

    // -----------------------------------------------------------------------------
    // clip
    // -----------------------------------------------------------------------------
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] =  L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }
    
    if (n == 3)
    {
        L[3] = L[0];
    }

    if (n == 4)
    {
        L[4] = L[0];
    }
}

// -----------------------------------------------------------------------------

vec3 EvaluateLTC(in vec3 _WSNormal, in vec3 _WSViewDirection, in vec3 _WSPosition, in mat3 _MinV, in vec3 _Points[4], in bool _TwoSided)
{
    float Sum;
    vec3  L[5];
    vec3  T1, T2;
    int   ClipIndex;

    // -----------------------------------------------------------------------------
    // construct orthonormal basis around N
    // -----------------------------------------------------------------------------
    T1 = normalize(_WSViewDirection - _WSNormal * dot(_WSViewDirection, _WSNormal));
    T2 = cross(_WSNormal, T1);

    // -----------------------------------------------------------------------------
    // rotate area light in (T1, T2, N) basis
    // -----------------------------------------------------------------------------
    _MinV = _MinV * (transpose(mat3(T1, T2, _WSNormal)));

    // -----------------------------------------------------------------------------
    // polygon (allocate 5 vertices for clipping)
    // -----------------------------------------------------------------------------
    L[0] = _MinV * (_Points[0] - _WSPosition);
    L[1] = _MinV * (_Points[1] - _WSPosition);
    L[2] = _MinV * (_Points[2] - _WSPosition);
    L[3] = _MinV * (_Points[3] - _WSPosition);

    ClipIndex = 0;

    ClipQuadToHorizon(L, ClipIndex);
    
    if (ClipIndex == 0)
    {
        return vec3(0.0f, 0.0f, 0.0f);
    }

    // -----------------------------------------------------------------------------
    // project onto sphere
    // -----------------------------------------------------------------------------
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    L[4] = normalize(L[4]);

    // -----------------------------------------------------------------------------
    // integrate
    // -----------------------------------------------------------------------------
    Sum = 0.0f;

    Sum += IntegrateEdge(L[0], L[1]);
    Sum += IntegrateEdge(L[1], L[2]);
    Sum += IntegrateEdge(L[2], L[3]);

    if (ClipIndex >= 4)
    {
        Sum += IntegrateEdge(L[3], L[4]);
    }

    if (ClipIndex == 5)
    {
        Sum += IntegrateEdge(L[4], L[0]);
    }

    Sum = _TwoSided ? abs(Sum) : max(0.0, Sum);

    return vec3(Sum, Sum, Sum);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0    , in_TexCoord);
    vec4  GBuffer1 = texture(ps_GBuffer1    , in_TexCoord);
    vec4  GBuffer2 = texture(ps_GBuffer2    , in_TexCoord);
    float VSDepth  = texture(ps_DepthTexture, in_TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_TexCoord, g_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (g_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

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
    // Create default rect in world
    // -----------------------------------------------------------------------------
    SRectangle Lightbulb;
    vec3  LightbulbCorners[4];
    
    vec3 LightDirection = -normalize(vec3(-2.0f, -2.0f, -1.0f));
    vec3 Left           =  normalize(vec3(0.0f, rotz, 1.0f));
    vec3 Right          =  normalize(cross(LightDirection, Left));
    
    Left = cross(LightDirection, Right);
    
    Lightbulb.m_DirectionX = Right;
    Lightbulb.m_DirectionY = Left;
    
    Lightbulb.m_Center = vec3(0.0f, 0.0f, 10.0f);
    Lightbulb.m_HalfWidth  = 0.5f * 8;
    Lightbulb.m_HalfHeight  = 0.5f * 8;

    Lightbulb.m_Plane = vec4(LightDirection, -dot(LightDirection, Lightbulb.m_Center));

    vec3 ex = Lightbulb.m_HalfWidth  * Lightbulb.m_DirectionX;
    vec3 ey = Lightbulb.m_HalfHeight * Lightbulb.m_DirectionY;

    LightbulbCorners[0] = Lightbulb.m_Center - ex - ey;
    LightbulbCorners[1] = Lightbulb.m_Center + ex - ey;
    LightbulbCorners[2] = Lightbulb.m_Center + ex + ey;
    LightbulbCorners[3] = Lightbulb.m_Center - ex + ey;

    // -----------------------------------------------------------------------------
    // LTC
    // -----------------------------------------------------------------------------
    vec3 Output = vec3(0.0f);

    vec3 WSViewDirection = normalize(g_ViewPosition.xyz - Data.m_WSPosition);
    
    float Theta = acos(dot(Data.m_WSNormal, WSViewDirection));

    vec2 UV = vec2(Data.m_Roughness, Theta / (0.5f * PI));

    UV = UV * LUT_SCALE + LUT_BIAS;
    
    vec4 LTCMaterial = texture2D(ps_LTCMaterial, UV);

    mat3 LTCMatrix = mat3(
        vec3(1.0f         , 0.0f         , LTCMaterial.y),
        vec3(0.0f         , LTCMaterial.z, 0.0f         ),
        vec3(LTCMaterial.w, 0.0f         , LTCMaterial.x)
    );
    
    vec3 Specular = EvaluateLTC(Data.m_WSNormal, WSViewDirection, WSPosition, LTCMatrix, LightbulbCorners, false);

    Specular *= texture2D(ps_LTCMag, UV).w;
    
    vec3 Diffuse = EvaluateLTC(Data.m_WSNormal, WSViewDirection, WSPosition, mat3(1), LightbulbCorners, false); 
    
    Output  = vec3(intensity) * (scolor.xyz * Specular + dcolor.xyz * Diffuse);

    Output /= 2.0f * PI;

    // -----------------------------------------------------------------------------
    // Check if light bulb is visible
    // -----------------------------------------------------------------------------
    SRay Ray;

    Ray.m_Origin    = g_ViewPosition.xyz;
    Ray.m_Direction = -WSViewDirection.xyz;

    float DistanceToGround = length(Data.m_WSPosition - g_ViewPosition.xyz);

    float DistanceToLightbulb = 0.0f;

    if (RayRectangleIntersect(Ray, Lightbulb, DistanceToLightbulb))
    {
        if (DistanceToLightbulb < DistanceToGround)
        {
            Output = vec3(intensity);
        }
    }

    out_Output = vec4(Output * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__