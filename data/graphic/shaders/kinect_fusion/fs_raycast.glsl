
#ifndef __INCLUDE_FS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_FS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"
#include "common_raycast.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0) uniform isampler3D fs_Volume;

layout(location = 0) in vec3 in_WSRayDirection;

void main()
{
    vec3 WSPosition = GetPosition(g_ViewPosition.xyz, normalize(in_WSRayDirection), fs_Volume);

    if (WSPosition.x != 0.0f)
    {
        vec3 WSNormal = GetNormal(WSPosition, fs_Volume);
        gl_FragColor = vec4(WSNormal * 0.5f + 0.5f, 1.0f);
    }
    else
    {
        discard;
    }
}

#endif // __INCLUDE_FS_KINECT_RAYCAST_GLSL__