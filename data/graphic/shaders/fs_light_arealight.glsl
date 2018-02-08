
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__

#include "common.glsl"
#include "common_global.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
const float LUT_SIZE  = 32.0f;
const float LUT_SCALE = (LUT_SIZE - 1.0f) / LUT_SIZE;
const float LUT_BIAS  = 0.5f / LUT_SIZE;

layout(std140, binding = 1) uniform UB1
{
    vec4  m_Color;
    vec4  m_Position;
    vec4  m_DirectionX;
    vec4  m_DirectionY;
    vec4  m_Plane;
    float m_HalfWidth;
    float m_HalfHeight;
    float m_IsTwoSided;
    float m_IsTextured;
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

layout(binding = 6) uniform sampler2D ps_FilteredMap;
// Input
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to light accumulation target
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Linearly Transformed Cosines
// -----------------------------------------------------------------------------
float Function3(vec3 CurrentEdgeVertex, vec3 PreviousEdgeVertex)
{
    float Temp = clamp(dot (CurrentEdgeVertex, PreviousEdgeVertex), -0.9999f, 0.9999f);

    return PI / 2.0f - (sign(Temp) * (PI / 2.0f - (sqrt((1.0f - abs(Temp))) * (PI / 2.0f + (abs(Temp) * (-0.2146018f + (abs(Temp) * (0.08656672f + (abs(Temp) * -0.03102955f)))))))));
}

vec3 Function2(vec3 _NextEdge, vec3 _CurrentEdge)
{
    vec3 Return;

    Return.z = 0.0f;
    Return.x = ((-(_NextEdge.z) * _CurrentEdge.x) + (_CurrentEdge.z * _NextEdge.x));
    Return.y = ((-(_NextEdge.z) * _CurrentEdge.y) + (_CurrentEdge.z * _NextEdge.y));

    return normalize(Return);
}

vec3 Function1(mat3 _LCTMatrix, vec3 _V1, vec3 _T1, vec3 _T2, vec3 _WSNormal)
{
    vec3 Return;

    Return.x = dot (_V1, _T1);
    Return.y = dot (_V1, _T2);
    Return.z = dot (_V1, _WSNormal);

    return _LCTMatrix * Return;
}

void CalculateUVAndLOD(vec3 _p0_27, vec3 _p0_24, vec3 _p0_32, out vec2 _UV, out float _LOD)
{
    vec3 A = (_p0_27 - _p0_24);
    vec3 B = (_p0_32 - _p0_24);

    vec3 C = ((A.yzx * B.zxy) - (A.zxy * B.yzx));

    float CdotC = dot (C, C);
    float tmpvar_40 = dot (C, _p0_24);

    vec3 D = (((tmpvar_40 * C) / CdotC) - _p0_24);

    float AdotB        = dot (A, B);
    float InverseAdotA = (1.0 / (dot (A, A)));

    vec3 E = (B - ((A * AdotB) * InverseAdotA));

    _UV.y = (dot (E, D) / dot (E, E));
    _UV.x = ((dot (A, D) * InverseAdotA) - ((AdotB * InverseAdotA) * _UV.y));

    _UV  = (vec2(0.125f, 0.125f) + (0.75f * _UV));
    _LOD = (log((vec2(textureSize(ps_FilteredMap, 0)).x * (abs(tmpvar_40) / pow(CdotC, 0.75f)))) / 1.098612f);
}


vec3 EvaluateLTC(in SSurfaceData _Data, in vec3 _WSViewDirection, in mat3 _MinV, in vec3 _Points[4], in bool _TwoSided, in bool _Textured)
{
    vec3 NextEdgeVertex;
    vec3 CurrentEdgeVertex;
    vec3 PreviousEdgeVertex;
    vec3  L[4];
    vec2  FilteredUV;
    float FilteredLOD;
    float Sum;
  
    // -----------------------------------------------------------------------------
    // Construct orthonormal basis around world-space normal
    // -----------------------------------------------------------------------------
    vec3 T1 = normalize((_WSViewDirection - (_Data.m_WSNormal * dot (_WSViewDirection, _Data.m_WSNormal))));
    vec3 T2 = normalize(((_Data.m_WSNormal.yzx * T1.zxy) - (_Data.m_WSNormal.zxy * T1.yzx)));

    vec3 WSWorldToCorner;

    WSWorldToCorner = (_Points[0].xyz - _Data.m_WSPosition);

    vec3 p0_24 = Function1(_MinV, WSWorldToCorner, T1, T2, _Data.m_WSNormal);
    L[0] = normalize(p0_24);

    WSWorldToCorner = (_Points[1].xyz - _Data.m_WSPosition);

    vec3 p0_27  = Function1(_MinV, WSWorldToCorner, T1, T2, _Data.m_WSNormal);
    L[1] = normalize(p0_27);

    WSWorldToCorner = (_Points[2].xyz - _Data.m_WSPosition);

    L[2] = normalize(Function1(_MinV, WSWorldToCorner, T1, T2, _Data.m_WSNormal));

    WSWorldToCorner = (_Points[3].xyz - _Data.m_WSPosition);

    vec3 p0_32  = Function1(_MinV, WSWorldToCorner, T1, T2, _Data.m_WSNormal);
    L[3] = normalize(p0_32);


    vec4 LightTexture = vec4(1.0f);

    if (_Textured)
    {
        CalculateUVAndLOD(p0_27, p0_24, p0_32, FilteredUV, FilteredLOD);

        LightTexture = textureLod(ps_FilteredMap, FilteredUV, FilteredLOD);    
    }

    // -----------------------------------------------------------------------------
    // detect clipping config
    // -----------------------------------------------------------------------------
    int Config = 0;

    if ((L[0].z < 0.0)) 
    {
        Config = 1;

        if ((L[1].z < 0.0f)) 
        {
            Config = 2;

            if ((L[2].z < 0.0f)) 
            {
                Config = 3;

                if ((L[3].z < 0.0f)) 
                {
                    Config = 4;
                }
            }
        }
    }

    Sum = 0.0f;
  
    if ((Config < 4)) 
    {
        PreviousEdgeVertex = L[Config];
        NextEdgeVertex     = L[Config];

        for (int IndexOfConfig = 1; IndexOfConfig <= 4; IndexOfConfig++) 
        {
            CurrentEdgeVertex = NextEdgeVertex;

            int Config = int((float(mod (float((Config + IndexOfConfig)), 4.0f))));

            NextEdgeVertex = L[Config];

            if ((CurrentEdgeVertex.z >= 0.0f)) 
            {
                if (NextEdgeVertex.z >= 0.0f)
                {
                    PreviousEdgeVertex = NextEdgeVertex;
                }
                else
                {
                    PreviousEdgeVertex = Function2(NextEdgeVertex, CurrentEdgeVertex);
                }

                float tmpvar_54 = Function3(CurrentEdgeVertex, PreviousEdgeVertex);

                Sum = (Sum + ((tmpvar_54 * ((CurrentEdgeVertex.yzx * PreviousEdgeVertex.zxy) - (CurrentEdgeVertex.zxy * PreviousEdgeVertex.yzx)).z) / sin(tmpvar_54)));
            } 
            else 
            {
                if ((NextEdgeVertex.z > 0.0f)) 
                {
                    CurrentEdgeVertex = Function2(-NextEdgeVertex, CurrentEdgeVertex);

                    float tmpvar_58 = Function3(PreviousEdgeVertex, CurrentEdgeVertex);

                    Sum = (Sum + ((tmpvar_58 * ((PreviousEdgeVertex.yzx * CurrentEdgeVertex.zxy) - (PreviousEdgeVertex.zxy * CurrentEdgeVertex.yzx)).z) / sin(tmpvar_58)));

                    PreviousEdgeVertex = NextEdgeVertex;

                    float tmpvar_60 = Function3(CurrentEdgeVertex, NextEdgeVertex);

                    Sum = (Sum + ((tmpvar_60 * ((CurrentEdgeVertex.yzx * NextEdgeVertex.zxy) - (CurrentEdgeVertex.zxy * NextEdgeVertex.yzx)).z) / sin(tmpvar_60)));
                }   
            }
        }

        Sum = _TwoSided ? abs(Sum) : max(0.0f, Sum);
    }

    return vec3(Sum) * LightTexture.xyz;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0    , in_UV);
    vec4  GBuffer1 = texture(ps_GBuffer1    , in_UV);
    vec4  GBuffer2 = texture(ps_GBuffer2    , in_UV);
    float VSDepth  = texture(ps_DepthTexture, in_UV).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_UV, g_ScreenToView);
    
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
    // Create lightbulb in world based on rectangle
    // -----------------------------------------------------------------------------
    SRectangle Lightbulb;

    Lightbulb.m_Center     = m_Position.xyz;
    Lightbulb.m_DirectionX = m_DirectionX.xyz;
    Lightbulb.m_DirectionY = m_DirectionY.xyz;
    Lightbulb.m_HalfWidth  = m_HalfWidth;
    Lightbulb.m_HalfHeight = m_HalfHeight;
    Lightbulb.m_Plane      = m_Plane;

    vec3 LightbulbCorners[4];

    vec3 ExtendX = Lightbulb.m_HalfWidth  * Lightbulb.m_DirectionX;
    vec3 ExtendY = Lightbulb.m_HalfHeight * Lightbulb.m_DirectionY;

    LightbulbCorners[0] = Lightbulb.m_Center - ExtendX - ExtendY;
    LightbulbCorners[1] = Lightbulb.m_Center + ExtendX - ExtendY;
    LightbulbCorners[2] = Lightbulb.m_Center + ExtendX + ExtendY;
    LightbulbCorners[3] = Lightbulb.m_Center - ExtendX + ExtendY;

    // -----------------------------------------------------------------------------
    // LTC
    // -----------------------------------------------------------------------------
    vec3 Luminance = vec3(0.0f);

    vec3 WSViewDirection = normalize(g_ViewPosition.xyz - Data.m_WSPosition);
    
    float NdotV = dot(Data.m_WSNormal, WSViewDirection);
    float Theta = acos(NdotV);

    vec2 UV = vec2(Data.m_Roughness, Theta / (0.5f * PI)) * LUT_SCALE + LUT_BIAS;
        
    vec4 LTCMaterial = texture(ps_LTCMaterial, UV);

    mat3 LTCMatrix = mat3(
        vec3(1.0f         , 0.0f         , LTCMaterial.y),
        vec3(0.0f         , LTCMaterial.z, 0.0f         ),
        vec3(LTCMaterial.w, 0.0f         , LTCMaterial.x)
    );
        
    vec3 Specular = EvaluateLTC(Data, WSViewDirection, LTCMatrix, LightbulbCorners, m_IsTwoSided > 0.0f, m_IsTextured > 0.0f);

    Specular = Specular * 4.0f * Data.m_SpecularAlbedo * texture(ps_LTCMag, UV).x;
    
    vec3 Diffuse = EvaluateLTC(Data, WSViewDirection, mat3(1.0f), LightbulbCorners, m_IsTwoSided > 0.0f, m_IsTextured > 0.0f); 
    
    Diffuse = Diffuse * 4.0f * Data.m_DiffuseAlbedo * texture(ps_LTCMag, UV).x;
    
    Luminance = m_Color.xyz * (Specular + Diffuse) / (2.0f * PI);

    // -----------------------------------------------------------------------------
    // Check if light bulb is visible
    // -----------------------------------------------------------------------------
    out_Output = vec4(Luminance * AverageExposure, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__