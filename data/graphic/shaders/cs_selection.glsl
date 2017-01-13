#ifndef __INCLUDE_CS_PICKING_BLUR_GLSL__
#define __INCLUDE_CS_PICKING_BLUR_GLSL__

#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -------------------------------------------------------------------------------------
// Defines
// -------------------------------------------------------------------------------------
#define TILE_SIZE 1

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std430, binding = 0) readonly buffer USettingBuffer
{
    uint m_MinX;
    uint m_MinY;
    uint m_MaxX;
    uint m_MaxY;
};

layout(std430, binding = 1) writeonly buffer UOutput
{
    vec4  out_WSPosition;
    vec4  out_WSNormal;
    float out_Depth;
    uint  out_EntityID;
};

layout (binding = 0, rgba8) readonly uniform image2D cs_GBuffer0;
layout (binding = 1, rgba8) readonly uniform image2D cs_GBuffer1;
layout (binding = 2, rgba8) readonly uniform image2D cs_GBuffer2;
layout (binding = 3) uniform sampler2D cs_Depth;
layout (binding = 4, r32ui) readonly uniform uimage2D cs_HitProxy;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;


    // -----------------------------------------------------------------------------
    // Check the current pixel and find information about this including:
    // 1. GBuffer data
    // 2. Entity ID
    // -----------------------------------------------------------------------------

    // ...

    // -----------------------------------------------------------------------------
    // Wait until every compute unit has finished.
    // -----------------------------------------------------------------------------
    
    // memoryBarrier();
    

    // -----------------------------------------------------------------------------
    // Use the best result from the compute units.
    // -----------------------------------------------------------------------------

    // ...


    // -----------------------------------------------------------------------------
    // That implementation is for a first test and will be replaced as soon
    // as needed.
    // -----------------------------------------------------------------------------
    ivec2 UV = ivec2(m_MinX, m_MinY);

    vec2 HomogeneousUV = UV * g_InvertedScreensizeAndScreensize.xy;

    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = imageLoad(cs_GBuffer0, UV);
    vec4  GBuffer1 = imageLoad(cs_GBuffer1, UV);
    vec4  GBuffer2 = imageLoad(cs_GBuffer2, UV);
    float VSDepth  = texture2D(cs_Depth   , HomogeneousUV).x;
    uint  ID       = imageLoad(cs_HitProxy, UV).x;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, HomogeneousUV, g_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (g_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    UnpackGBuffer(GBuffer0, GBuffer1, GBuffer2, WSPosition.xyz, VSDepth, Data);

    // -----------------------------------------------------------------------------
    // Return data
    // -----------------------------------------------------------------------------
    out_WSPosition = vec4(Data.m_WSPosition, 1.0f);
    out_WSNormal   = vec4(Data.m_WSNormal, 0.0f);
    out_Depth      = Data.m_VSDepth;
    out_EntityID   = ID;
}

#endif // __INCLUDE_CS_PICKING_BLUR_GLSL__