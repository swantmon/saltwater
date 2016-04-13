
#ifndef __INCLUDE_COMMON_GBUFFER_GLSL_
#define __INCLUDE_COMMON_GBUFFER_GLSL_

#include "common.glsl"
#include "common_material.glsl"

// -----------------------------------------------------------------------------
// Data
// -----------------------------------------------------------------------------
struct SGBuffer
{
    vec4 m_Color0;
    vec4 m_Color1;
    vec4 m_Color2;
};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
void PackGBuffer(in vec3 _BaseColor, in vec3 _WSNormal, in float _Roughness, in vec3 _Reflectance, in float _MetalMask, in float _AO, out SGBuffer _GBuffer)
{
    vec3  PackedWSNormal = PackNormal(normalize(_WSNormal));
    vec3  BaseColor      = _BaseColor * (1.0f - _MetalMask);
    vec3  F0             = _BaseColor * _MetalMask + 0.16f * _Reflectance * _Reflectance * (1.0f - _MetalMask);
    float Roughness      = clamp(_Roughness, 0.0f, 1.0f);

    _GBuffer.m_Color0 = vec4(PackedWSNormal.xy, Roughness, 0.0f);
    _GBuffer.m_Color1 = vec4(BaseColor, PackedWSNormal.z);
    _GBuffer.m_Color2 = vec4(F0, _AO);
}

// -----------------------------------------------------------------------------

void UnpackGBuffer(in vec4 _GBuffer0, in vec4 _GBuffer1, in vec4 _GBuffer2, in vec3 _WSPosition, in float _VSDepth, out SSurfaceData _Data)
{
    // -----------------------------------------------------------------------------
    // Read data from texture
    // -----------------------------------------------------------------------------
    vec4 GBuffer0 = _GBuffer0;
    vec4 GBuffer1 = _GBuffer1;
    vec4 GBuffer2 = _GBuffer2;
    
    // -----------------------------------------------------------------------------
    // Depth
    // -----------------------------------------------------------------------------
    _Data.m_VSDepth = _VSDepth;
    
    // -----------------------------------------------------------------------------
    // World-space position
    // -----------------------------------------------------------------------------
    _Data.m_WSPosition = _WSPosition;
    
    // -----------------------------------------------------------------------------
    // Normal
    // -----------------------------------------------------------------------------
    _Data.m_WSNormal = normalize(UnpackNormal(GBuffer0.rg, GBuffer1.a));
    
    // -----------------------------------------------------------------------------
    // Diffuse albedo
    // -----------------------------------------------------------------------------
    _Data.m_DiffuseAlbedo = GBuffer1.rgb;
    
    // -----------------------------------------------------------------------------
    // Specular albedo
    // -----------------------------------------------------------------------------
    _Data.m_SpecularAlbedo = GBuffer2.rgb;
    
    // -----------------------------------------------------------------------------
    // Roughness
    // -----------------------------------------------------------------------------
    _Data.m_Roughness = GBuffer0.b;
    
    // -----------------------------------------------------------------------------
    // Ambient Occlusion
    // -----------------------------------------------------------------------------
    _Data.m_AmbientOcclusion = GBuffer2.a;
}

#endif // __INCLUDE_COMMON_GBUFFER_GLSL_