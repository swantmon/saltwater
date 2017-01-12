#ifndef __INCLUDE_CS_PICKING_BLUR_GLSL__
#define __INCLUDE_CS_PICKING_BLUR_GLSL__

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define TILE_SIZE 1

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer USettingBuffer
{
    ivec2 in_UV;
};

layout(std430, binding = 1) writeonly buffer UOutput
{
    vec4  out_WSPosition;
    vec4  out_WSNormal;
    float out_Depth;
};

layout (binding = 0, rgba8) readonly uniform image2D cs_GBuffer0;
layout (binding = 1, rgba8) readonly uniform image2D cs_GBuffer1;
layout (binding = 2, rgba8) readonly uniform image2D cs_GBuffer2;
layout (binding = 3, r32f)  readonly uniform image2D cs_Depth;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;

    vec4  GBuffer0 = imageLoad(cs_GBuffer0, in_UV);
    vec4  GBuffer1 = imageLoad(cs_GBuffer1, in_UV);
    vec4  GBuffer2 = imageLoad(cs_GBuffer2, in_UV);
    float Depth    = imageLoad(cs_Depth, in_UV).x;

    out_WSPosition = GBuffer0;
    out_WSNormal   = GBuffer1;
    out_Depth      = Depth;
}

#endif // __INCLUDE_CS_PICKING_BLUR_GLSL__