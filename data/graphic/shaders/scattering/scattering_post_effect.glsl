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
    float3 g_SunDirection;
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

const static float HeightOffset = 0.01f;

float GetLinearDepth(float _Depth)
{
    float VSZ = g_Near / (g_Far - ((g_Far - g_Near) * _Depth));
    return VSZ;
}

float3 GetWSPosition(float3 _WSViewRay, float _LinearDepth)
{
    return _LinearDepth * _WSViewRay + g_WSEyePositionPS;
}

float3 GetInscatterColor(float _FogDepth, float3 _X, float _T, float3 _V, float3 _S, float _Radius, float _Mu, out float3 _Attenuation, bool _IsSceneGeometry) 
{
    float3 Result = float3(0.0f, 0.0f, 0.0f); 	// X in space and ray looking in space, intialize
	_Attenuation = float3(1.0f, 1.0f, 1.0f);

	float D = -_Radius * _Mu - sqrt(_Radius * _Radius * (_Mu * _Mu - 1.0f) + g_RadiusAtmosphere * g_RadiusAtmosphere);
    if (D > 0.0f) 
	{ 
		// if X in space and ray intersects atmosphere
        // move X to nearest intersection of ray with top atmosphere boundary
        _X += D * _V;
        _T -= D;
        _Mu = (_Radius * _Mu + D) / g_RadiusAtmosphere;
        _Radius = g_RadiusAtmosphere;
    }

	float Epsilon = 0.005f;

	if (_Radius < g_RadiusGround + HeightOffset + Epsilon)
	{
		float Diff = (g_RadiusGround + HeightOffset + Epsilon) - _Radius;
		_X -= Diff * _V;
		_T -= Diff;
		_Radius = g_RadiusGround + HeightOffset + Epsilon;
		_Mu = dot(_X, _V) / _Radius;
	}

    if (_Radius <= g_RadiusAtmosphere && _FogDepth > 0.0f)
	{ 
		float3 X0 = _X + _T * _V;
		float R0 = length(X0);
		// if ray intersects atmosphere
		float Nu = dot(_V, _S);
		float MuS = dot(_X, _S) / _Radius;

		float MuHorizon = -sqrt(1.0f - (g_RadiusGround / _Radius) * (g_RadiusGround / _Radius));

		if (_IsSceneGeometry)
		{
			_Mu = max(_Mu, MuHorizon + Epsilon + 0.15f);
		}
		else
		{ 
			_Mu = max(_Mu, MuHorizon + Epsilon);
		}

		float MuOriginal = _Mu;

		float BlendRatio = 0.0f;

		if (_IsSceneGeometry)
		{
			BlendRatio = saturate(exp(-_V.z) - 0.5f);
			if (BlendRatio < 1.0f)
			{
				_V.z = max(_V.z, 0.15f);
				_V = normalize(_V);
				float3 X1 = _X + _T * _V;
				float R1 = length(X1);
				_Mu = dot(X1, _V) / R1; 
			}
		}

		float PhaseR = PhaseFunctionR(Nu);
		float PhaseM = PhaseFunctionM(Nu);
		float4 Inscatter = max(Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu), 0.0f);

		if (_T > 0.0f) 
		{
			_Attenuation = GetAnalyticTransmittance(_Radius, _Mu, _T);

			float Mu0 = dot(X0, _V) / R0;
			float MuS0 = dot(X0, _S) / R0;

			if (_IsSceneGeometry)
			{
				R0 = max(R0, _Radius);
			}

			if (R0 > g_RadiusGround + HeightOffset)
			{
				if (BlendRatio < 1.0f)
				{
					Inscatter = max(Inscatter - _Attenuation.rgbr * Texture4DSample(g_InscatterTable, R0, Mu0, MuS0, Nu), 0.0f);
					if (!_IsSceneGeometry ) 
					{
						if (abs(_Mu - MuHorizon) < Epsilon)
						{
							float Alpha = ((_Mu - MuHorizon) + Epsilon) * 0.5f / Epsilon;

							_Mu = MuHorizon - Epsilon;
							R0 = sqrt(_Radius * _Radius + _T * _T + 2.0f * _Radius * _T * _Mu);
							Mu0 = (_Radius * _Mu + _T) / R0;

							Mu0 = max(MuHorizon + Epsilon, Mu0);
							float4 Inscatter0 = Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu);
							float4 Inscatter1 = Texture4DSample(g_InscatterTable, R0, Mu0, MuS0, Nu);
							float4 InscatterA = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);

							_Mu = MuHorizon + Epsilon;
							R0 = sqrt(_Radius * _Radius + _T * _T + 2.0f * _Radius * _T * _Mu);

							Mu0 = (_Radius * _Mu + _T) / R0;
							Mu0 = max(MuHorizon + Epsilon, Mu0);
							Inscatter0 = Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu);
							Inscatter1 = Texture4DSample(g_InscatterTable, R0, Mu0, MuS0, Nu);
							float4 InscatterB = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);

							Inscatter = lerp(InscatterA, InscatterB, Alpha);
						}
					}
					else if (BlendRatio > 0.0f)
					{
						Inscatter = lerp(Inscatter,
							(1.0f - _Attenuation.rgbr) * max(Texture4DSample(g_InscatterTable, _Radius, MuOriginal, MuS, Nu), 0.0f), 
							BlendRatio);
					}
				}
				else
				{
					Inscatter = (1.0f - _Attenuation.rgbr) * Inscatter; 
				}
			}
		}
        Inscatter.w *= smoothstep(0.0f, 0.02f, MuS);
        Result = max(Inscatter.rgb * PhaseR + GetMie(Inscatter) * PhaseM, 0.0f);
    } 

	return Result;
}

float3 GetGroundColor(float4 _SceneColor, float3 _X, float _T, float3 _V, float3 _S, float _Radius, float3 _Attenuation, bool _IsSceneGeometry)
{
    float3 Result = float3(0.0f, 0.0f, 0.0f); 	// ray looking at the sky (for intial value)
    
    if (_T > 0.0f)
	{ 
        float3 X0 = _X + _T * _V;
        float R0 = length(X0);
        float3 N = X0 / R0;
		
		_SceneColor.xyz = saturate(_SceneColor.xyz + 0.05f);

        float4 Reflectance = _SceneColor * float4(0.2f, 0.2f, 0.2f, 1.0f);

        float MuS = dot(N, _S);

        float3 SunLight = _IsSceneGeometry ? float3(0.0f, 0.0f, 0.0f) : GetTransmittanceWithShadow(R0, MuS);
        //float3 SunLight = GetTransmittanceWithShadow(R0, MuS);
        float3 GroundSkyLight = GetIrradiance(g_IrradianceTable, R0, MuS);
        float3 GroundColor = (Reflectance.rgb * (max(MuS, 0.0f) * SunLight + GroundSkyLight)) / g_PI;

        Result = GroundColor * _Attenuation;
    }

    return Result;
}

float3 GetSunColor(float3 _X, float _T, float3 _V, float3 _S, float _Radius, float _Mu) 
{
	float3 TransmittanceValue = _Radius <= g_RadiusAtmosphere ? GetTransmittanceWithShadow(_Radius, _Mu) : float3(1.0f, 1.0f, 1.0f);
    if (_T > 0.0f) 
	{
        return float3(0.0f, 0.0f, 0.0f);
    }
	else 
	{
		float SunIntensity = step(cos(g_PI * 1.0f / 180.0f), dot(_V, _S));
        return TransmittanceValue * SunIntensity;
    }
}

float4 GetAtmosphericFog(float3 _Position, float3 _ViewDirection, float _Depth, float3 _SceneColor)
{
    bool IsSceneGeometry = _Depth < g_Far;

	float Scale = 0.00001f; // meters to km
	_Position.y = _Position.y + 100000.0f;  // ground offset
	_Position *= Scale;
	_Position.y += g_RadiusGround + HeightOffset;

	float Radius = length(_Position);

	float Mu = dot(_Position, _ViewDirection) / Radius;

	float T = -Radius * Mu - sqrt(Radius * Radius * (Mu * Mu - 1.0f) + g_RadiusGround * g_RadiusGround);

	float DepthThreshold = 100.0f; // 100km limit
	_Depth *= Scale;
    
	float FogDepth = max(0.0f, _Depth - 15000.0f);
	float ShadowFactor = 1.0f; // shadow approximation
	float DistanceRatio = min(FogDepth * 0.1f / 15000.0f, 1.0f);

    //bool IsSceneGeometry = (_Depth < DepthThreshold); // Assume as scene geometry
	
    if (IsSceneGeometry)
	{
		ShadowFactor = DistanceRatio * 1.0f; // TODO: fog multiplier? density multiplier?
		T = max(_Depth, 1.0f);
	}

    //return IsSceneGeometry;

	float3 Attenuation = 1.0f;

	float3 InscatterColor = GetInscatterColor(1, _Position, T, _ViewDirection, g_SunDirection, Radius, Mu, Attenuation, IsSceneGeometry); // TODO: understand FogDepth
    float3 GroundColor = GetGroundColor(float4(_SceneColor, 1.0f), _Position, T, _ViewDirection, g_SunDirection, Radius, Attenuation, IsSceneGeometry);
    float3 Sun = GetSunColor(_Position, T, _ViewDirection, g_SunDirection, Radius, Mu);

    //return float4(Attenuation, 1.0f);
    //return float4(GroundColor + InscatterColor + Sun, 1.0f);

	// Decay color
	float3 OriginalColor = Sun + GroundColor + InscatterColor;
    return float4(OriginalColor, 1.0f);
	OriginalColor = ShadowFactor * OriginalColor * float3(1.0f, 1.0f, 1.0f); // sun color
	float4 OutColor = float4(OriginalColor, lerp(saturate(Attenuation.r), 1.0f, (1.0f - DistanceRatio)));
    return OutColor;
}

float4 PSShader(PSInput _Input) : SV_Target
{
    float2 UV = _Input.m_CSPosition.xy;

    float CSDepth = g_DepthBuffer[UV];
    float LinearDepth = GetLinearDepth(CSDepth);
    float SceneDepth = LinearDepth * g_Far;
    
    float3 WSViewDirection = normalize(_Input.m_WSViewRay);
    float3 WSPosition = GetWSPosition(_Input.m_WSViewRay, LinearDepth);
    
    float4 Color = GetAtmosphericFog(WSPosition, WSViewDirection, SceneDepth, g_FrameBuffer[UV].rgb);
    return Color;
}