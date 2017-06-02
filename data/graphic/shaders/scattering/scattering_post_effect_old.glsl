////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Scattering post effect shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <scattering_common.fx>

Texture2D<float4> g_FrameBuffer        : register(t3);
Texture2D<float4> g_NormalBuffer       : register(t4);
Texture2D<float>  g_DepthBuffer        : register(t5);

const static float g_EpsilonInscatter = 0.004f;

cbuffer VSBuffer : register(b0)
{
    float4x4 g_InvViewProjectionMatrix;
    float3 g_WSEyePositionVS;
}

cbuffer PSBuffer : register(b1)
{
    float g_Near;
    float g_Far;
    float3 g_WSEyePositionPS;
    float3 g_SunDir;
    float3 g_SunIntensity;
}

struct PSInput
{
    float4 m_CSPosition : SV_Position;
    float3 m_WSViewRay : TEXCOORD;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Vertex shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PSInput VSShader(uint _VertexID : SV_VertexID)
{
    float2 Vertices[] =
    {
        float2(-1.0f, -1.0f),
        float2( 3.0f, -1.0f),
        float2(-1.0f,  3.0f),
    };

    float4 CSPosition = float4(Vertices[_VertexID], 0.0f, 1.0f);
    float4 CSViewRayEnd = float4(Vertices[_VertexID], 1.0f, 1.0f);

    float4 WSViewRayEnd = mul(CSViewRayEnd, g_InvViewProjectionMatrix);
    WSViewRayEnd /= WSViewRayEnd.w;

    PSInput Output;

    Output.m_CSPosition = CSPosition;
    Output.m_WSViewRay = WSViewRayEnd.xyz - g_WSEyePositionVS;

    return Output;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// Pixel shader
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float GetLinearDepth(float2 _TexCoords)
{
    float CSZ = g_DepthBuffer[_TexCoords];

    float VSZ = g_Near / (g_Far - ((g_Far - g_Near) * CSZ));

    return VSZ;
}

float3 GetWSPosition(float2 _TexCoords, float3 _WSViewRay)
{
    float Depth = GetLinearDepth(_TexCoords);

    return Depth * _WSViewRay + g_WSEyePositionPS;
}

bool IntersectAtmosphere(float3 _ViewDir, out float _EyeOffset, out float _AtmospherePath)
{
    float3 WSEyePosition = g_WSEyePositionPS;
    //WSEyePosition.y = WSEyePosition.y / 1000.0f + g_RadiusGround;

    _EyeOffset = 0.0f;
    _AtmospherePath = 0.0f;

    // vector from ray origin to sphere center
    float3 L = -WSEyePosition;
    float L2 = dot(L, L);
    float S = dot(L, _ViewDir);

    float R = g_RadiusGround - 0.00001f; // prevent artifacts
    float R2 = R * R;

    if (L2 <= R2)
    {
        float M2 = L2 - (S * S);
        float Q = sqrt(R2 - M2);
        _AtmospherePath = S + Q;

        return true;
    }
    else if (S >= 0)
    {
        float M2 = L2 - (S * S);

        if (M2 <= R2)
        {
            float Q = sqrt(R2 - M2);
            _EyeOffset = S - Q;
            _AtmospherePath = (S + Q) - _EyeOffset;

            return true;
        }
    }

    return false;
}

float3 GetInscatteredLight(float3 _WSViewDir, float3 _WSPosition, out float3 _Attenuation, out float _IrradianceFactor)
{
    float3 WSEyePosition = g_WSEyePositionPS;
    //WSEyePosition.y = WSEyePosition.y / 1000.0f + g_RadiusGround;

    _Attenuation = 1.0f;
    _IrradianceFactor = 0.0f;

    float3 InscatteredLight = 0.0f;

    float EyeOffset = 0.0f;
    float MaxAtmospherePath = 0.0f; // not considering occlusion

    if (IntersectAtmosphere(_WSViewDir, EyeOffset, MaxAtmospherePath))
    {
        float PathLength = distance(WSEyePosition, _WSPosition);
        if (PathLength > EyeOffset) // put camera on top of atmosphere when in space
        {
            float3 RayStart = WSEyePosition + EyeOffset * _WSViewDir;
            float StartHeight = length(RayStart); // g_RadiusGround?
            PathLength -= EyeOffset;

            float Mu = dot(RayStart, _WSViewDir) / StartHeight;
            float Nu = dot(_WSViewDir, g_SunDir);
            float MuS = dot(RayStart, g_SunDir) / StartHeight;

            float4 Inscatter = max(Texture4DSample(g_InscatterTable, StartHeight, Mu, MuS, Nu), 0.0f);
            float EndHeight = length(_WSPosition);
            float MuSEnd = dot(_WSPosition, g_SunDir) / EndHeight;

            if (PathLength < MaxAtmospherePath)
            {
                _Attenuation = GetAnalyticTransmittance(StartHeight, MuS, PathLength);
                float MuEnd = dot(_WSPosition, _WSViewDir) / EndHeight;
                float4 InscatterSurface = Texture4DSample(g_InscatterTable, EndHeight, MuEnd, MuSEnd, Nu);
                Inscatter = max(Inscatter - _Attenuation.rgbr * InscatterSurface, 0.0f);
                _IrradianceFactor = 1.0f;
            }
            else
            {
                _Attenuation = GetAnalyticTransmittance(StartHeight, Mu, PathLength);
            }

            float MuHorizon = -sqrt(1.0f - (g_RadiusGround / StartHeight) * (g_RadiusGround / StartHeight));

            if (abs(Mu - MuHorizon) < g_EpsilonInscatter)
            {
                float Mu = MuHorizon - g_EpsilonInscatter;
                float SampleHeight = sqrt(StartHeight * StartHeight + PathLength * PathLength + 2.0f * StartHeight * PathLength * Mu);
                float MuSamplePos = (StartHeight * Mu + PathLength) / SampleHeight;
                float4 Inscatter0 = Texture4DSample(g_InscatterTable, StartHeight, Mu, MuS, Nu);
                float4 Inscatter1 = Texture4DSample(g_InscatterTable, SampleHeight, MuSamplePos, MuSEnd, Nu);
                float4 InscatterA = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);
                Mu = MuHorizon + g_EpsilonInscatter;
                SampleHeight = sqrt(StartHeight * StartHeight + PathLength * PathLength + 2.0f * StartHeight * PathLength * Mu);
                MuSamplePos = (StartHeight * Mu + PathLength) / SampleHeight;
                Inscatter0 = Texture4DSample(g_InscatterTable, StartHeight, Mu, MuS, Nu);
                Inscatter1 = Texture4DSample(g_InscatterTable, SampleHeight, MuSamplePos, MuSEnd, Nu);
                float4 InscatterB = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);
                float T = ((Mu - MuHorizon) + g_EpsilonInscatter) / (2.0f * g_EpsilonInscatter);
                Inscatter = lerp(InscatterA, InscatterB, T);
            }
            Inscatter.w *= smoothstep(0.0f, 0.02f, MuS);
            float PhaseRayleigh = PhaseFunctionR(Nu);
            float PhaseMie = PhaseFunctionM(Nu);
            InscatteredLight = max(Inscatter.rgb * PhaseRayleigh + GetMie(Inscatter) * PhaseMie, 0.0f);
            InscatteredLight *= g_SunIntensity;
        }
    }
    return InscatteredLight;
}

float3 GetReflectedLight(float3 _WSPosition, float2 _UV, float3 _Attenuation, float _IrradianceFactor)
{
    _Attenuation = 1.0f;
    _IrradianceFactor = 0.0f;

    float3 WSNormal = normalize(g_NormalBuffer[_UV].rgb);
    float3 Color = g_FrameBuffer[_UV].rgb;
    
    float3 SunDir = normalize(g_SunDir);
    float3 LightIntensity = g_SunIntensity;
    float LightScale = max(dot(WSNormal, SunDir), 0.0f);
    float Height = length(_WSPosition);
    float MuS = dot(_WSPosition, SunDir) / Height;
    float3 Irradiance = GetIrradiance(g_IrradianceTable, Height, MuS) * _IrradianceFactor;
    float3 AttenuationSunLight = GetTransmittance(Height, MuS);
    float3 ReflectedLight = Color * (LightScale * AttenuationSunLight + Irradiance) * LightIntensity;
    ReflectedLight *= _Attenuation;
    return ReflectedLight;
}

float4 PSShader(PSInput _Input) : SV_Target
{
    float3 WSViewDirection = normalize(_Input.m_WSViewRay);
    float3 WSPosition = GetWSPosition(_Input.m_CSPosition.xy, WSViewDirection);
    //WSPosition.y = WSPosition.y / 1000.0f + g_RadiusGround;

    float3 Attenuation = 1.0f;
    float IrradianceFactor = 0.0f;

    float3 InscatteredLight = GetInscatteredLight(WSViewDirection, WSPosition, Attenuation, IrradianceFactor);
    float3 ReflectedLight = GetReflectedLight(WSPosition, _Input.m_CSPosition.xy, Attenuation, IrradianceFactor);

    float4 Color = float4(ReflectedLight + InscatteredLight, 1.0f);
    return Color;
}