#ifndef __INCLUDE_FS_HCB_GENERATION_GLSL__
#define __INCLUDE_FS_HCB_GENERATION_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 0) uniform UB0
{
    vec4 ps_ConstantBufferData0;
    vec4 ps_ConstantBufferData1;
};

uniform sampler2D ps_Texture0;

// -----------------------------------------------------------------------------
// Easy access defines
// -----------------------------------------------------------------------------
#define ps_UVBoundaries       ps_ConstantBufferData0
#define ps_InverseTextureSize ps_ConstantBufferData1.xy
#define ps_MipmapLevel        ps_ConstantBufferData1.z

// -----------------------------------------------------------------------------
// Input to fragment from VS
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_UV;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main(void)
{
    if (ps_MipmapLevel == 0.0f)
    {
        out_Output  = textureLod(ps_Texture0, in_UV, 0);
    }
    else
    {
        const vec2 InUVMinBound = ps_UVBoundaries.xy;
        const vec2 InUVMaxBound = ps_UVBoundaries.zw;
        
        const vec2 Radius = 2.0 * ps_InverseTextureSize;
        // sigma = 0.5
        const float C = 0.130371;
        const float A = 0.115349;
        const float B = 0.102059;

        out_Output  = textureLod(ps_Texture0, clamp(in_UV, InUVMinBound, InUVMaxBound), 0) * C;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1,  1) * Radius, InUVMinBound, InUVMaxBound), 0) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 0,  1) * Radius, InUVMinBound, InUVMaxBound), 0) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1,  1) * Radius, InUVMinBound, InUVMaxBound), 0) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1,  0) * Radius, InUVMinBound, InUVMaxBound), 0) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1, -1) * Radius, InUVMinBound, InUVMaxBound), 0) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 0, -1) * Radius, InUVMinBound, InUVMaxBound), 0) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1, -1) * Radius, InUVMinBound, InUVMaxBound), 0) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1,  0) * Radius, InUVMinBound, InUVMaxBound), 0) * A;
    }
}

#endif // __INCLUDE_FS_HCB_GENERATION_GLSL__