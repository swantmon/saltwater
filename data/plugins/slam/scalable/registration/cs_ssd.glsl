#ifndef __INCLUDE_CS_SSD_GLSL__
#define __INCLUDE_CS_SSD_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    float ErrorSum = 0.0f;

    for (int x = Coords.x - 2; x <= Coords.x + 2; ++ x)
    {
        for (int y = Coords.y - 2; y <= Coords.y + 2; ++ y)
        {
            vec3 Target = imageLoad(cs_Target, ivec2(x, y)).rgb;
            vec3 Source = imageLoad(cs_Source, ivec2(x, y)).rgb;

            float TargetIntensity = (Target.x + Target.y + Target.z) / 3.0f;
            float SourceIntensity = (Source.x + Source.y + Source.z) / 3.0f;

            float Error = (TargetIntensity - SourceIntensity);
            ErrorSum += Error * Error;
        }
    }

    imageStore(cs_SSD, Coords, vec4(ErrorSum));
}

#endif //__INCLUDE_CS_SSD_GLSL__