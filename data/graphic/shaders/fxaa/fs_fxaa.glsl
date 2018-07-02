
#ifndef __INCLUDE_FS_FXAA_GLSL__
#define __INCLUDE_FS_FXAA_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Defines / Settings
// -----------------------------------------------------------------------------
float FXAA_SPAN_MAX   = 8.0f;
float FXAA_REDUCE_MUL = 1.0f / 8.0f;
float FXAA_REDUCE_MIN = 1.0f / 128.0f;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
uniform sampler2D ps_InputTexture;
uniform sampler2D PSTextureDepth;

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// More info:
// http://horde3d.org/wiki/index.php5?title=Shading_Technique_-_FXAA
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// http://www.ngohq.com/images/articles/fxaa/FXAA_WhitePaper.pdf
// -----------------------------------------------------------------------------
void main(void)
{
    // -----------------------------------------------------------------------------
    // Get a set of neighbor pixels
    // -----------------------------------------------------------------------------
    vec3 rgbNW = texture(ps_InputTexture, in_UV.xy + (vec2(-1.0f, -1.0f) * g_InvertedScreensizeAndScreensize.xy)).xyz;
    vec3 rgbNE = texture(ps_InputTexture, in_UV.xy + (vec2(+1.0f, -1.0f) * g_InvertedScreensizeAndScreensize.xy)).xyz;
    vec3 rgbSW = texture(ps_InputTexture, in_UV.xy + (vec2(-1.0f, +1.0f) * g_InvertedScreensizeAndScreensize.xy)).xyz;
    vec3 rgbSE = texture(ps_InputTexture, in_UV.xy + (vec2(+1.0f, +1.0f) * g_InvertedScreensizeAndScreensize.xy)).xyz;
    vec3 rgbM  = texture(ps_InputTexture, in_UV.xy).xyz;
    
    // -----------------------------------------------------------------------------
    // Calculate luminanz on pixels
    // -----------------------------------------------------------------------------
    vec3 Luma = vec3(0.299, 0.587, 0.144);
    
    float LumaNW = dot(rgbNW, Luma);
    float LumaNE = dot(rgbNE, Luma);
    float LumaSW = dot(rgbSW, Luma);
    float LumaSE = dot(rgbSE, Luma);
    float LumaM  = dot(rgbM , Luma);
    
    // -----------------------------------------------------------------------------
    // Find min and max luminanz in pixelset
    // -----------------------------------------------------------------------------
    float LumaMin = min(LumaM, min(min(LumaNW, LumaNE), min(LumaSW, LumaSE)));
    float LumaMax = max(LumaM, max(max(LumaNW, LumaNE), max(LumaSW, LumaSE)));
    
    // -----------------------------------------------------------------------------
    // Compute direction
    // -----------------------------------------------------------------------------
    vec2 Direction;
    Direction.x = -((LumaNW + LumaNE) - (LumaSW + LumaSE));
    Direction.y =  ((LumaNW + LumaSW) - (LumaNE + LumaSE));
    
    // -----------------------------------------------------------------------------
    // Compute a direction reduce
    // -----------------------------------------------------------------------------
    float DirectionReduce = max( (LumaNW + LumaNE + LumaSW + LumaSE) * (0.25f * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN );
    
    // -----------------------------------------------------------------------------
    // Compute reduction and build reciprocal
    // -----------------------------------------------------------------------------
    float ReciprocalDirection = 1.0f / (min(abs(Direction.x), abs(Direction.y)) + DirectionReduce);
    
    // -----------------------------------------------------------------------------
    // Get final direction
    // -----------------------------------------------------------------------------
    Direction = min( vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max( vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), Direction * ReciprocalDirection ) ) * g_InvertedScreensizeAndScreensize.xy;
    
    // -----------------------------------------------------------------------------
    // Now we get a interpolation between neighbor pixels
    // -----------------------------------------------------------------------------
    vec3 rgbA = 0.5f * (texture(ps_InputTexture, in_UV.xy + Direction * (1.0f / 3.0f - 0.5f)).xyz + texture(ps_InputTexture, in_UV.xy + Direction * (2.0f / 3.0f - 0.5f)).xyz);
    
    vec3 rgbB = rgbA * 0.5f + 0.25f * (texture(ps_InputTexture, in_UV.xy + Direction * (0.0f / 3.0f - 0.5f)).xyz + texture(ps_InputTexture, in_UV.xy + Direction * (3.0f / 3.0f - 0.5f)).xyz);
    
    // -----------------------------------------------------------------------------
    // Compute a luminanze on second rgb value
    // -----------------------------------------------------------------------------
    float LumaB = dot(rgbB, Luma);
    
    // -----------------------------------------------------------------------------
    // Find best matching color
    // -----------------------------------------------------------------------------
    if((LumaB < LumaMin) || (LumaB > LumaMax))
    {
        out_Output = vec4(rgbA, 1.0f);
    }
    else
    {
        out_Output = vec4(rgbB, 1.0f);
    }
}

#endif // __INCLUDE_FS_FXAA_GLSL__