
#ifndef __INCLUDE_COMMON_GLSL__
#define __INCLUDE_COMMON_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#define PI      3.1415926535897932f
#define INV_PI  1.0f / PI
#define F16_MAX 65503.0f

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec4 InverseToneMapping(in vec4 _Color)
{
    float Luminosity = 0.3f * _Color.r + 0.59f * _Color.g + 0.11f * _Color.b;

    float LuminosityScale = 10 * pow(Luminosity, 10.0f) + 1.8f;

    return LuminosityScale * _Color;
}

// -----------------------------------------------------------------------------

vec3 GetViewSpacePositionFromDepth(in float _Depth, in vec2 _ScreenPosition, in mat4 _InvertedProjectionMatrix)
{
    // -----------------------------------------------------------------------------
    // Information from:
    // http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
    //
    // We need to multiply the depth result by 2 and subtract 1 to convert the
    // range to [-1, 1], just as you're doing already for the xy components of
    // screenSpacePosition. Then we'll have the proper NDC position, and you can
    // apply the inverse view-projection matrix as you're doing.
    // -----------------------------------------------------------------------------
    vec4 ScreenPosition;
    
    ScreenPosition.x = _ScreenPosition.x * 2.0f - 1.0f;
    ScreenPosition.y = _ScreenPosition.y * 2.0f - 1.0f;
    ScreenPosition.z = _Depth;
    ScreenPosition.w = 1.0f;
    
    // -----------------------------------------------------------------------------
    // Transform by the inverse projection matrix
    // -----------------------------------------------------------------------------
    vec4 VSPosition = _InvertedProjectionMatrix * ScreenPosition;
    
    // -----------------------------------------------------------------------------
    // Divide by w to get the view-space position
    // -----------------------------------------------------------------------------
    return (VSPosition.xyz / VSPosition.w);
}

// -----------------------------------------------------------------------------

float ConvertToLinearDepth(in float _HyperbolicDepth, in float _Near, in float _Far)
{
    return (2.0f * _Near) / (_Far + _Near - (_HyperbolicDepth * 2.0f - 1.0f) * (_Far - _Near));
}

// -----------------------------------------------------------------------------

float ConvertToHyperbolicDepth(in float _LinearDepth, in mat4 _ProjectionMatrix)
{
    float LinearDepth = -_LinearDepth;
    
    return (_ProjectionMatrix[2].z * LinearDepth + _ProjectionMatrix[3].z) / (-LinearDepth);
}

// -----------------------------------------------------------------------------

vec3 PackNormal(in vec3 _UnpackedNormal)
{
    return _UnpackedNormal * 0.5f + 0.5f;
}

// -----------------------------------------------------------------------------

vec3 UnpackNormal(in vec2 _PackedNormalXY, in float _PackedNormalZ)
{
    return vec3(_PackedNormalXY, _PackedNormalZ) * 2.0f - 1.0f;
}

// -----------------------------------------------------------------------------

float GetRadicalInverseVdC(in uint _Bits)
{
    _Bits = (_Bits << 16u) | (_Bits >> 16u);
    _Bits = ((_Bits & 0x55555555u) << 1u) | ((_Bits & 0xAAAAAAAAu) >> 1u);
    _Bits = ((_Bits & 0x33333333u) << 2u) | ((_Bits & 0xCCCCCCCCu) >> 2u);
    _Bits = ((_Bits & 0x0F0F0F0Fu) << 4u) | ((_Bits & 0xF0F0F0F0u) >> 4u);
    _Bits = ((_Bits & 0x00FF00FFu) << 8u) | ((_Bits & 0xFF00FF00u) >> 8u);
    
    return float(_Bits) * 2.3283064365386963e-10; // / 0x100000000
}

// -----------------------------------------------------------------------------

vec2 GetHammersley(uint _Index, uint _Sum)
{
    return vec2(float(_Index)/float(_Sum), GetRadicalInverseVdC(_Index));
}

// -----------------------------------------------------------------------------

void CalcWorldBase(in vec3 _NormalUnitDir, out vec3 _BaseX, out vec3 _BaseY, out vec3 _BaseZ)
{
    vec3 NonCollinearAxis;
    
    // -----------------------------------------------------------------------------
    // Choose an axis of the world system which is not collinear to the normal.
    // -----------------------------------------------------------------------------
    NonCollinearAxis = (abs(_NormalUnitDir.y) < 0.999f) ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f);
    
    // -----------------------------------------------------------------------------
    // Calculate the world base.
    // -----------------------------------------------------------------------------
    _BaseZ = _NormalUnitDir;
    _BaseX = normalize(cross(NonCollinearAxis, _BaseZ));
    _BaseY = cross(_BaseZ, _BaseX);
}

#endif // __INCLUDE_COMMON_GLSL__