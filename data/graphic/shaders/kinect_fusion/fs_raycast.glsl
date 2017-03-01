
#ifndef __INCLUDE_FS_KINECT_RAYCAST_GLSL__
#define __INCLUDE_FS_KINECT_RAYCAST_GLSL__

#include "common_global.glsl"
#include "common_tracking.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0) uniform isampler3D cs_Volume;

layout(location = 0) in vec3 in_WSRayDirection;

void main()
{
    gl_FragColor = vec4(in_WSRayDirection, 1.0f);
}

#endif // __INCLUDE_FS_KINECT_RAYCAST_GLSL__