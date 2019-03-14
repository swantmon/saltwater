#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, rgba8) uniform image2D cs_Diminished;
layout (binding = 1, rgba8) uniform image2D cs_MembranePatches;
layout (binding = 2, rgba8) uniform image2D cs_MembraneBorders;

shared int IsOverlapping;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    if (gl_LocalInvocationID == 0)
    {
        IsOverlapping = 0;
    }

    barrier();

    const ivec2 MembraneCoords = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 ImageCoords = ivec2(vec2(MembraneCoords) / imageSize(cs_MembranePatches) * imageSize(cs_Diminished));

    imageStore(cs_MembraneBorders, MembraneCoords, vec4(0.0f));

    vec3 Color = imageLoad(cs_Diminished, ImageCoords).rgb;

    if (dot(Color, vec3(1.0)) != 0.0)
    {
        atomicAdd(IsOverlapping, 1);
    }

    barrier();
    
    if (IsOverlapping > 0)
    {
        imageStore(cs_MembranePatches, MembraneCoords, vec4(1.0f));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__