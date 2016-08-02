
#ifndef __INCLUDE_FS_DOF_DOWN_SAMPLE_GLSL__
#define __INCLUDE_FS_DOF_DOWN_SAMPLE_GLSL__

#include "common.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 1) uniform UB1
{
    vec2 m_DofNear;
    vec2 m_DofRowDelta;
};

layout(binding = 0) uniform sampler2D PSTextureColor;
layout(binding = 1) uniform sampler2D PSTextureDepth;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
struct SPixelCoords
{
    vec2 m_Color0;
    vec2 m_Color1;
    vec2 m_Depth0;
    vec2 m_Depth1;
    vec2 m_Depth2;
    vec2 m_Depth3;
};

layout(location = 0) in SPixelCoords in_PixelCoords;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;


void main(void)
{
    // -----------------------------------------------------------------------------
    // Initialize variables
    // CoC = Circle of Confusion
    // More info at: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch28.html
    // -----------------------------------------------------------------------------
    vec3  Color;
    float MaxCoc;
    vec4  Depth;
    vec4  CurCoc;
    vec4  Coc;
    vec2  RowOfs[4];
    
    // -----------------------------------------------------------------------------
    // Calculate row offset
    // -----------------------------------------------------------------------------
    RowOfs[0] = vec2(0.0f);
    RowOfs[1] = m_DofRowDelta.xy;
    RowOfs[2] = m_DofRowDelta.xy * 2.0f;
    RowOfs[3] = m_DofRowDelta.xy * 3.0f;
    
    // -----------------------------------------------------------------------------
    // Bilinear filtering to average 4 color samples
    // -----------------------------------------------------------------------------
    Color  = vec3(0.0f);
    Color += texture(PSTextureColor, in_PixelCoords.m_Color0.xy + RowOfs[0]).rgb;
    Color += texture(PSTextureColor, in_PixelCoords.m_Color1.xy + RowOfs[0]).rgb;
    Color += texture(PSTextureColor, in_PixelCoords.m_Color0.xy + RowOfs[2]).rgb;
    Color += texture(PSTextureColor, in_PixelCoords.m_Color1.xy + RowOfs[2]).rgb;
    Color /= 4.0f;
    
    // -----------------------------------------------------------------------------
    // Calculate CoC
    // -----------------------------------------------------------------------------
    Depth.x = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth0.xy + RowOfs[0]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.y = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth1.xy + RowOfs[0]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.z = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth2.xy + RowOfs[0]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.w = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth3.xy + RowOfs[0]).r, g_CameraParameterNear, g_CameraParameterFar);
    
    CurCoc = clamp(m_DofNear.x * Depth + m_DofNear.y, 0.0f, 1.0f);
    Coc    = CurCoc;
    
    // -----------------------------------------------------------------------------
    
    Depth.x = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth0.xy + RowOfs[1]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.y = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth1.xy + RowOfs[1]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.z = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth2.xy + RowOfs[1]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.w = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth3.xy + RowOfs[1]).r, g_CameraParameterNear, g_CameraParameterFar);
    
    CurCoc = clamp(m_DofNear.x * Depth + m_DofNear.y, 0.0f, 1.0f);
    Coc    = max(Coc, CurCoc);
    
    // -----------------------------------------------------------------------------
    
    Depth.x = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth0.xy + RowOfs[2]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.y = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth1.xy + RowOfs[2]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.z = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth2.xy + RowOfs[2]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.w = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth3.xy + RowOfs[2]).r, g_CameraParameterNear, g_CameraParameterFar);
    
    CurCoc = clamp(m_DofNear.x * Depth + m_DofNear.y, 0.0f, 1.0f);
    Coc    = max(Coc, CurCoc);
    
    // -----------------------------------------------------------------------------
    
    Depth.x = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth0.xy + RowOfs[3]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.y = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth1.xy + RowOfs[3]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.z = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth2.xy + RowOfs[3]).r, g_CameraParameterNear, g_CameraParameterFar);
    Depth.w = ConvertToLinearDepth(texture(PSTextureDepth, in_PixelCoords.m_Depth3.xy + RowOfs[3]).r, g_CameraParameterNear, g_CameraParameterFar);
    
    CurCoc = clamp(m_DofNear.x * Depth + m_DofNear.y, 0.0f, 1.0f);
    Coc    = max(Coc, CurCoc);
    
    // -----------------------------------------------------------------------------
    // Get maximal Coc
    // -----------------------------------------------------------------------------
    MaxCoc = max( max( Coc[0], Coc[1] ), max( Coc[2], Coc[3] ) );
    
    // -----------------------------------------------------------------------------
    // Return final down sampled image
    // -----------------------------------------------------------------------------
    out_Output = vec4(Color, MaxCoc);
}

#endif // __INCLUDE_FS_DOF_DOWN_SAMPLE_GLSL__