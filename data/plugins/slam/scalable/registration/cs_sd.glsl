#ifndef __INCLUDE_CS_SSD_GLSL__
#define __INCLUDE_CS_SSD_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
    
    vec3 Target = imageLoad(cs_Target, Coords).rgb;
    vec3 Source = imageLoad(cs_Source, ivec2(Coords + g_Offset)).rgb;

    float TargetIntensity = (Target.x + Target.y + Target.z) / 3.0f;
    float SourceIntensity = (Source.x + Source.y + Source.z) / 3.0f;

    float Error = (TargetIntensity - SourceIntensity);

    imageStore(cs_SSD, Coords, vec4(Error * Error));
}

#endif //__INCLUDE_CS_SSD_GLSL__