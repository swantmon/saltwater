
#ifndef __INCLUDE_COMMON_MATERIAL_GLSL__
#define __INCLUDE_COMMON_MATERIAL_GLSL__

// -----------------------------------------------------------------------------
// Data
// -----------------------------------------------------------------------------
struct SSurfaceData
{
    float m_VSDepth;
    vec3  m_WSPosition;
    vec3  m_WSNormal;
    vec3  m_DiffuseAlbedo;
    vec3  m_SpecularAlbedo;
    float m_Roughness;
    float m_AmbientOcclusion;
};

// -----------------------------------------------------------------------------
// Roughness
// -----------------------------------------------------------------------------
float GetClampedRoughness(in float _Roughness)
{
    // -----------------------------------------------------------------------------
    // The smallest normalized value that can be represented in IEEE 754 (FP16) 
    // is 2^-14 = 6.1e-5 = 0,00006103515625
    // The code will make the following computation involving roughness: 
    // 1.0 / Roughness^4.
    //
    // Note that we also clamp to 1.0 to match the deferred renderer on PC where 
    // the roughness is stored in an 8-bit value and thus automatically clamped 
    // at 1.0.
    // Increase value from 0.04 to 0.12 to fix missing specular lobe problem 
    // on device
    // -----------------------------------------------------------------------------
    return clamp(_Roughness, 0.12f, 1.0f);
}

// -----------------------------------------------------------------------------

float GetMipLevelByRoughness(in float _Roughness, in float _NumberOfMipLevels)
{
    return _Roughness * _NumberOfMipLevels;
}

// -----------------------------------------------------------------------------

float AdjustRoughness(in float _InputRoughness, in float _AverageNormalLength)
{
    // -----------------------------------------------------------------------------
    // Based on The Order: 1886 SIGGRAPH course notes implementation
    // -----------------------------------------------------------------------------
    if (_AverageNormalLength < 1.0f)
    {
        float AverageNormLength2 = _AverageNormalLength * _AverageNormalLength;
        float Kappa = (3.0f * _AverageNormalLength - _AverageNormalLength * AverageNormLength2) / (1.0f - AverageNormLength2);
        
        float Variance = 1.0f / Kappa;
        
        return sqrt(_InputRoughness * _InputRoughness + Variance);
    }

    return (_InputRoughness);
}

#endif // __INCLUDE_COMMON_MATERIAL_GLSL__