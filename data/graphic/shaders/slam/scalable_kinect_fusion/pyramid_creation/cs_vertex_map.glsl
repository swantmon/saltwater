
#ifndef __INCLUDE_CS_VERTEX_MAP_GLSL__
#define __INCLUDE_CS_VERTEX_MAP_GLSL__

#include "slam/common_tracking.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_DepthBuffer;
layout (binding = 1, MAP_TEXTURE_FORMAT) writeonly uniform image2D cs_VertexMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const ivec2 ImageSize = imageSize(cs_DepthBuffer);
    
    const int PyramidLevel = int(log2(DEPTH_IMAGE_WIDTH / ImageSize.x));

    const vec2 ImagePos = vec2(gl_GlobalInvocationID.xy);
    
    const vec2 FocalPoint = g_Intrinsics[PyramidLevel].m_FocalPoint;
    const vec2 InvFocalLength = g_Intrinsics[PyramidLevel].m_InvFocalLength;

    const ivec2 DepthPos = ivec2(ImageSize.x - ImagePos.x, ImagePos.y);
    const float Depth = imageLoad(cs_DepthBuffer, DepthPos).x / 1000.0f;
    
    vec4 Vertex = vec4(0.0f);   
    
    bool IsValid = true;

    for (int x = -2; x <= 2; ++ x)
    {
        for (int y = -2; y <= 2; ++ y)
        {
            float Sample = imageLoad(cs_DepthBuffer, DepthPos + ivec2(x, y)).x / 1000.0f;

            if (Sample < 0.5f)
            {
                IsValid = false;
            }
        }
    }

    Vertex.xy = Depth * (ImagePos - FocalPoint) * InvFocalLength;
    Vertex.z = Depth;
    Vertex.w = 1.0f;

    imageStore(cs_VertexMap, ivec2(ImagePos), IsValid ? Vertex : vec4(0.0f));
}

#endif // __INCLUDE_CS_VERTEX_MAP_GLSL__