#ifndef __INCLUDE_CS_SHIFT_DEPTH_GLSL__
#define __INCLUDE_CS_SHIFT_DEPTH_GLSL__

layout(binding = 0, rgba8ui) readonly uniform uimage2D cs_Source;
layout(binding = 1, rgba8ui) writeonly uniform uimage2D cs_Target;

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    ivec4 Source = ivec4(imageLoad(cs_Source, Coords));
    ivec4 Target = ivec4(imageLoad(cs_Target, Coords));
}

#endif //__INCLUDE_CS_SHIFT_DEPTH_GLSL__