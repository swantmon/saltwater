
#ifndef __INCLUDE_FS_TONE_MAPPING_GLSL__
#define __INCLUDE_FS_TONE_MAPPING_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 0) uniform UColorProperties
{
    vec4 ps_ColorMatrixR_ColorCurveCd1;
    vec4 ps_ColorMatrixG_ColorCurveCd3Cm3;
    vec4 ps_ColorMatrixB_ColorCurveCm2;
    vec4 ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3;
    vec4 ps_ColorCurve_Ch1_Ch2;
    vec4 ps_ColorShadow_Luma;
    vec4 ps_ColorShadow_Tint1;
    vec4 ps_ColorShadow_Tint2;
};

layout(binding = 0) uniform sampler2D ps_LightAccumulation;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Final;

// -----------------------------------------------------------------------------

vec3 GetFilmicToneMapping(vec3 _LinearColor) 
{
    vec3 CurveColor;
    vec3 MatrixColor;
    vec3 MatrixBrightColor;
    vec3 MatrixMidColor;
    vec3 MatrixDarkColor;
    vec3 BrightColor;
    vec3 MidColor;
    vec3 DarkColor;

    // -----------------------------------------------------------------------------
    // Color and exposure control.
    // Apply color matrix (channel mixer, exposure, saturation).
    // -----------------------------------------------------------------------------
    MatrixColor.r = dot(_LinearColor, ps_ColorMatrixR_ColorCurveCd1.rgb);
    MatrixColor.g = dot(_LinearColor, ps_ColorMatrixG_ColorCurveCd3Cm3.rgb);
    MatrixColor.b = dot(_LinearColor, ps_ColorMatrixB_ColorCurveCm2.rgb);

    MatrixColor *= ps_ColorShadow_Tint1.rgb + ps_ColorShadow_Tint2.rgb * inversesqrt(dot(_LinearColor, ps_ColorShadow_Luma.rgb) + vec3(1.0f));

    // -----------------------------------------------------------------------------
    // Required to insure saturation doesn't create negative colors!
    // -----------------------------------------------------------------------------
    MatrixColor = max(vec3(0.0f, 0.0f, 0.0f), MatrixColor);

    // -----------------------------------------------------------------------------
    // Full path.
    // -----------------------------------------------------------------------------
    MatrixBrightColor = max(MatrixColor, ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.zzz);
    MatrixMidColor    = clamp(MatrixColor, ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.xxx, ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.zzz);
    MatrixDarkColor   = max(vec3(0.0f), ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.xxx - MatrixColor);

    BrightColor       = (MatrixBrightColor * ps_ColorCurve_Ch1_Ch2.xxx + ps_ColorCurve_Ch1_Ch2.yyy) * inversesqrt(MatrixBrightColor + ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.www);
    MidColor          =  MatrixMidColor * ps_ColorMatrixB_ColorCurveCm2.aaa;
    DarkColor         = (MatrixDarkColor * ps_ColorMatrixR_ColorCurveCd1.aaa) * inversesqrt(MatrixDarkColor + ps_ColorCurve_Cm0Cd0_Cd2_Ch0Cm1_Ch3.yyy) + ps_ColorMatrixG_ColorCurveCd3Cm3.aaa;

    CurveColor        = BrightColor + MidColor + DarkColor;

    return CurveColor;
}

// -----------------------------------------------------------------------------

vec3 LinearToSRGB(in vec3 _LinearColor)
{
    vec3 sRGBLo;
    vec3 sRGBHi;
    vec3 sRGB;

    sRGBLo = _LinearColor * 12.92f;
    sRGBHi = (pow(abs(_LinearColor), vec3(1.0f / 2.4f)) * vec3(1.055f)) - vec3(0.055f);
    sRGB   = (_LinearColor.x <= 0.0031308f && _LinearColor.y <= 0.0031308f && _LinearColor.z <= 0.0031308f) ? sRGBLo : sRGBHi;

    return sRGB;
}

// -----------------------------------------------------------------------------

void main(void)
{
    vec3 LinearColor;
    vec3 ToneMappedLinearColor;
    vec3 Color;

    // -----------------------------------------------------------------------------
    // Read data from engine
    // -----------------------------------------------------------------------------
    LinearColor = texture(ps_LightAccumulation, in_TexCoord).rgb;

    // -----------------------------------------------------------------------------
    // Color Grading
    // -----------------------------------------------------------------------------
    ToneMappedLinearColor = GetFilmicToneMapping(LinearColor);
    Color                 = LinearToSRGB(ToneMappedLinearColor);

    // -----------------------------------------------------------------------------
    // Finalize
    // -----------------------------------------------------------------------------
    out_Final = vec4(Color, 1.0f);
}

#endif // __INCLUDE_FS_TONE_MAPPING_GLSL__