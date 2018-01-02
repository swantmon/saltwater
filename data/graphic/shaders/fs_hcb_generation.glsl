#ifndef __INCLUDE_FS_HCB_GENERATION_GLSL__
#define __INCLUDE_FS_HCB_GENERATION_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(row_major, std140, binding = 1) uniform UB1
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
        out_Output  = textureLod(ps_Texture0, in_UV, 0.0f);
    }
    else
    {
        vec2 InUVMinBound = ps_UVBoundaries.xy;
        vec2 InUVMaxBound = ps_UVBoundaries.zw;
        
        vec2 Radius = 2.0f * ps_InverseTextureSize;
        // sigma = 0.5f
        const float C = 0.130371;
        const float A = 0.115349;
        const float B = 0.102059;

        out_Output  = textureLod(ps_Texture0, clamp(in_UV, InUVMinBound, InUVMaxBound), 0.0f) * C;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1,  1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 0,  1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1,  1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1,  0) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2(-1, -1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 0, -1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * A;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1, -1) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * B;
        out_Output += textureLod(ps_Texture0, clamp(in_UV + vec2( 1,  0) * Radius, InUVMinBound, InUVMaxBound), 0.0f) * A;
    }
}

#endif // __INCLUDE_FS_HCB_GENERATION_GLSL__