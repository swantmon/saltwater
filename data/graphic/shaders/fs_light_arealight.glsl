
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
    vec4  m_Color;
    vec4  m_Position;
    vec4  m_DirectionX;
    vec4  m_DirectionY;
    vec4  m_Plane;
    float m_HalfWidth;
    float m_HalfHeight;
    float m_IsTwoSided;
    float m_Padding;
    uint  m_ExposureHistoryIndex;
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

vec3 EvaluateDiffuseLTC(in SSurfaceData _Data, in vec3 _WSViewDirection, in vec3 _Points[4], in bool _TwoSided)
{
    float Sum;
    vec3  L[5];
    vec3  T1, T2;
    int   ClipIndex;

    // -----------------------------------------------------------------------------
    // construct orthonormal basis around N
    // -----------------------------------------------------------------------------
    T1 = normalize(_WSViewDirection - _Data.m_WSNormal * dot(_WSViewDirection, _Data.m_WSNormal));
    T2 = cross(_Data.m_WSNormal, T1);

    // -----------------------------------------------------------------------------
    // polygon (allocate 5 vertices for clipping)
    // -----------------------------------------------------------------------------
    L[0] = _Points[0] - _Data.m_WSPosition;
    L[1] = _Points[1] - _Data.m_WSPosition;
    L[2] = _Points[2] - _Data.m_WSPosition;
    L[3] = _Points[3] - _Data.m_WSPosition;

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

    return m_Color.xyz * vec3(Sum, Sum, Sum) * _Data.m_DiffuseAlbedo;
}

// -----------------------------------------------------------------------------


vec3 EvaluateSpecularLTC(in SSurfaceData _Data, in vec3 _WSViewDirection, in mat3 _MinV, in vec3 _Points[4], in bool _TwoSided)
{
    float Sum;
    vec3  L[5];
    vec3  T1, T2;
    int   ClipIndex;

    // -----------------------------------------------------------------------------
    // construct orthonormal basis around N
    // -----------------------------------------------------------------------------
    T1 = normalize(_WSViewDirection - _Data.m_WSNormal * dot(_WSViewDirection, _Data.m_WSNormal));
    T2 = cross(_Data.m_WSNormal, T1);

    // -----------------------------------------------------------------------------
    // rotate area light in (T1, T2, N) basis
    // -----------------------------------------------------------------------------
    _MinV = _MinV * (transpose(mat3(T1, T2, _Data.m_WSNormal)));

    // -----------------------------------------------------------------------------
    // polygon (allocate 5 vertices for clipping)
    // -----------------------------------------------------------------------------
    L[0] = _MinV * (_Points[0] - _Data.m_WSPosition);
    L[1] = _MinV * (_Points[1] - _Data.m_WSPosition);
    L[2] = _MinV * (_Points[2] - _Data.m_WSPosition);
    L[3] = _MinV * (_Points[3] - _Data.m_WSPosition);

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

    return m_Color.xyz * vec3(Sum, Sum, Sum) * _Data.m_SpecularAlbedo;
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
    float AverageExposure = ps_ExposureHistory[m_ExposureHistoryIndex];

    // -----------------------------------------------------------------------------
    // Create default rect in world
    // -----------------------------------------------------------------------------
    SRectangle Lightbulb;
    vec3 LightbulbCorners[4];

    Lightbulb.m_Center     = m_Position.xyz;
    Lightbulb.m_DirectionX = m_DirectionX.xyz;
    Lightbulb.m_DirectionY = m_DirectionY.xyz;
    Lightbulb.m_HalfWidth  = m_HalfWidth;
    Lightbulb.m_HalfHeight = m_HalfHeight;
    Lightbulb.m_Plane      = m_Plane;

    vec3 ex = Lightbulb.m_HalfWidth  * Lightbulb.m_DirectionX;
    vec3 ey = Lightbulb.m_HalfHeight * Lightbulb.m_DirectionY;

    LightbulbCorners[0] = Lightbulb.m_Center - ex - ey;
    LightbulbCorners[1] = Lightbulb.m_Center + ex - ey;
    LightbulbCorners[2] = Lightbulb.m_Center + ex + ey;
    LightbulbCorners[3] = Lightbulb.m_Center - ex + ey;

    // -----------------------------------------------------------------------------
    // LTC
    // -----------------------------------------------------------------------------
    vec3 Luminance = vec3(0.0f);

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
    
    vec3 Specular = EvaluateSpecularLTC(Data, WSViewDirection, LTCMatrix, LightbulbCorners, m_IsTwoSided > 0.0f);

    Specular *= texture2D(ps_LTCMag, UV).w;
    
    vec3 Diffuse = EvaluateDiffuseLTC(Data, WSViewDirection, LightbulbCorners, m_IsTwoSided > 0.0f); 

    Diffuse = Diffuse / 2.0f * PI; 
    
    Luminance = Specular + Diffuse;

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
            Luminance = m_Color.xyz;
        }
    }

    out_Output = vec4(Luminance * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__