#ifndef __INCLUDE_FS_PAS_GLSL__
#define __INCLUDE_FS_PAS_GLSL__

#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"
#include "common_global.glsl"
#include "scattering/scattering_common.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
const float g_EpsilonInscatter = 0.004f;
const float HeightOffset = 0.01f;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 3) uniform UB2
{
    vec4 g_SunDirection;
    vec4 g_SunIntensity;
};

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Normal;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
float saturate(in float _Input)
{
    return clamp(_Input, 0.0f, 1.0f);
}

// -----------------------------------------------------------------------------

vec3 GetInscatterColor(float _FogDepth, vec3 _X, float _T, vec3 _V, vec3 _S, float _Radius, float _Mu, out vec3 _Attenuation, bool _IsSceneGeometry) 
{
    vec3 Result = vec3(0.0f, 0.0f, 0.0f); 	// X in space and ray looking in space, intialize
	_Attenuation = vec3(1.0f, 1.0f, 1.0f);

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
		vec3 X0 = _X + _T * _V;
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
				vec3 X1 = _X + _T * _V;
				float R1 = length(X1);
				_Mu = dot(X1, _V) / R1; 
			}
		}

		float PhaseR = PhaseFunctionR(Nu);
		float PhaseM = PhaseFunctionM(Nu);
		vec4 Inscatter = max(Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu), 0.0f);

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
							vec4 Inscatter0 = Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu);
							vec4 Inscatter1 = Texture4DSample(g_InscatterTable, R0, Mu0, MuS0, Nu);
							vec4 InscatterA = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);

							_Mu = MuHorizon + Epsilon;
							R0 = sqrt(_Radius * _Radius + _T * _T + 2.0f * _Radius * _T * _Mu);

							Mu0 = (_Radius * _Mu + _T) / R0;
							Mu0 = max(MuHorizon + Epsilon, Mu0);
							Inscatter0 = Texture4DSample(g_InscatterTable, _Radius, _Mu, MuS, Nu);
							Inscatter1 = Texture4DSample(g_InscatterTable, R0, Mu0, MuS0, Nu);
							vec4 InscatterB = max(Inscatter0 - _Attenuation.rgbr * Inscatter1, 0.0f);

							Inscatter = mix(InscatterA, InscatterB, Alpha);
						}
					}
					else if (BlendRatio > 0.0f)
					{
						Inscatter = mix(Inscatter,
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

// -----------------------------------------------------------------------------

vec3 GetGroundColor(vec4 _SceneColor, vec3 _X, float _T, vec3 _V, vec3 _S, float _Radius, vec3 _Attenuation, bool _IsSceneGeometry)
{
    vec3 Result = vec3(0.0f, 0.0f, 0.0f); 	// ray looking at the sky (for intial value)
    
    if (_T > 0.0f)
	{ 
        vec3 X0 = _X + _T * _V;
        float R0 = length(X0);
        vec3 N = X0 / R0;
		
		_SceneColor.xyz = clamp(_SceneColor.xyz + vec3(0.05f), vec3(0.0f), vec3(1.0f));

        vec4 Reflectance = _SceneColor * vec4(0.2f, 0.2f, 0.2f, 1.0f);

        float MuS = dot(N, _S);

        vec3 SunLight = _IsSceneGeometry ? vec3(0.0f, 0.0f, 0.0f) : GetTransmittanceWithShadow(R0, MuS);
        //vec3 SunLight = GetTransmittanceWithShadow(R0, MuS);
        vec3 GroundSkyLight = GetIrradiance(g_IrradianceTable, R0, MuS);
        vec3 GroundColor = (Reflectance.rgb * (max(MuS, 0.0f) * SunLight + GroundSkyLight)) / g_PI;

        Result = GroundColor * _Attenuation;
    }

    return Result;
}

// -----------------------------------------------------------------------------

vec3 GetSunColor(vec3 _X, float _T, vec3 _V, vec3 _S, float _Radius, float _Mu) 
{
	vec3 TransmittanceValue = _Radius <= g_RadiusAtmosphere ? GetTransmittanceWithShadow(_Radius, _Mu) : vec3(1.0f, 1.0f, 1.0f);

    if (_T > 0.0f) 
	{
        return vec3(0.0f, 0.0f, 0.0f);
    }
	else 
	{
		float SunIntensity = step(cos(g_PI * 1.0f / 180.0f), dot(_V, _S));

        return TransmittanceValue * SunIntensity;
    }
}

// -----------------------------------------------------------------------------

vec4 GetAtmosphericFog(vec3 _Position, vec3 _ViewDirection, float _Depth, vec3 _SceneColor)
{
    bool IsSceneGeometry = _Depth < g_CameraParameterFar;

	float Scale = 0.001f; // meters to km
	_Position.y = _Position.y + 1000.0f;  // ground offset
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

	vec3 Attenuation = vec3(1.0f);

	vec3 InscatterColor = GetInscatterColor(1.0f, _Position, T, _ViewDirection, normalize(g_SunDirection.xyz), Radius, Mu, Attenuation, IsSceneGeometry); // TODO: understand FogDepth
    vec3 GroundColor = GetGroundColor(vec4(_SceneColor, 1.0f), _Position, T, _ViewDirection, normalize(g_SunDirection.xyz), Radius, Attenuation, IsSceneGeometry);
    vec3 Sun = GetSunColor(_Position, T, _ViewDirection, normalize(g_SunDirection.xyz), Radius, Mu);

    //return vec4(Attenuation, 1.0f);
    //return vec4(Sun, 1.0f);

	// Decay color
	vec3 OriginalColor = Sun + GroundColor + InscatterColor;
    return vec4(OriginalColor, 1.0f);
	OriginalColor = ShadowFactor * OriginalColor * vec3(1.0f, 1.0f, 1.0f); // sun color
	vec4 OutColor = vec4(OriginalColor, mix(saturate(Attenuation.r), 1.0f, (1.0f - DistanceRatio)));
    return OutColor;
}

// -----------------------------------------------------------------------------

void main()
{        
    vec4 Color = GetAtmosphericFog(g_ViewPosition.xyz, normalize(in_Normal), 50000.0f, vec3(0.0f));

    out_Output = Color * g_SunIntensity;
}

#endif // __INCLUDE_FS_PAS_GLSL__