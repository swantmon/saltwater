
#ifndef __INCLUDE_CS_PLANE_EXTRACTION_GLSL__
#define __INCLUDE_CS_PLANE_EXTRACTION_GLSL__

layout(std430, binding = 0) buffer ExtractedPlanes
{
    int g_PlaneCount;
    ivec3 Padding;
    vec4 g_Planes[];
};

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r32i) uniform iimage2D cs_Histogram;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D cs_VertexMap;
layout (binding = 2, MAP_TEXTURE_FORMAT) uniform image2D cs_NormalMap;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{    
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);
    
    int Count = imageLoad(cs_Histogram, ivec2(x, y)).x;
}

#endif // __INCLUDE_CS_PLANE_EXTRACTION_GLSL__