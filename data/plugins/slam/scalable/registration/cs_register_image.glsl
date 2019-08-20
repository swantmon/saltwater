#ifndef __INCLUDE_CS_SHIFT_DEPTH_GLSL__
#define __INCLUDE_CS_SHIFT_DEPTH_GLSL__

layout(binding = 0, rgba8ui) uniform uimage2D cs_Target;
layout(binding = 1, rgba8ui) uniform uimage2D cs_Source;
layout(binding = 2, rgba32f) uniform image2D cs_Debug;

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    float ErrorSum = 0.0f;

    for (int x = Coords.x - 2; x <= Coords.x + 2; ++ x)
    {
        for (int y = Coords.y - 2; y <= Coords.y + 2; ++ y)
        {
            ivec3 Target = ivec3(imageLoad(cs_Target, ivec2(x, y))).rgb;
            ivec3 Source = ivec3(imageLoad(cs_Source, ivec2(x, y))).rgb;

            float TargetIntensity = (Target.x + Target.y + Target.z) / 3.0f;
            float SourceIntensity = (Source.x + Source.y + Source.z) / 3.0f;

            float Error = (TargetIntensity - SourceIntensity);
            ErrorSum += Error * Error;
        }
    }

    imageStore(cs_Debug, Coords, vec4(ErrorSum));
}

#endif //__INCLUDE_CS_SHIFT_DEPTH_GLSL__