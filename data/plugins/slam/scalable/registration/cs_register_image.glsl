#ifndef __INCLUDE_CS_SHIFT_DEPTH_GLSL__
#define __INCLUDE_CS_SHIFT_DEPTH_GLSL__

layout(binding = 0, rgba8ui) uniform uimage2D cs_Target;
layout(binding = 1, rgba8ui) uniform uimage2D cs_Source;
layout(binding = 2, rgba32f) uniform image2D cs_Debug;

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    ivec4 Target = ivec4(imageLoad(cs_Target, Coords));
    ivec4 Source = ivec4(imageLoad(cs_Source, Coords));
    vec4 Debug = imageLoad(cs_Debug, Coords);
}

#endif //__INCLUDE_CS_SHIFT_DEPTH_GLSL__