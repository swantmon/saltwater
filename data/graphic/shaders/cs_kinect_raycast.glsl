
#ifndef __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__
#define __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__

#include "tracking_common.glsl"

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, rg16ui) readonly uniform uimage3D cs_Volume;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_Vertex;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

float GetStartLength(vec3 Start, vec3 Direction)
{
    float xmin = ((Direction.x > 0.0 ? 0.0 : VOLUME_SIZE) - Start.x) / Direction.x;
    float ymin = ((Direction.y > 0.0 ? 0.0 : VOLUME_SIZE) - Start.y) / Direction.y;
    float zmin = ((Direction.z > 0.0 ? 0.0 : VOLUME_SIZE) - Start.z) / Direction.z;

    return max(max(xmin, ymin), zmin);
}

float GetEndLength(vec3 Start, vec3 Direction)
{
    float xmax = ((Direction.x > 0.0 ? VOLUME_SIZE : 0.0) - Start.x) / Direction.x;
    float ymax = ((Direction.y > 0.0 ? VOLUME_SIZE : 0.0) - Start.y) / Direction.y;
    float zmax = ((Direction.z > 0.0 ? VOLUME_SIZE : 0.0) - Start.z) / Direction.z;

    return min(min(xmax, ymax), zmax);
}

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    ivec2 VertexMapSize = imageSize(cs_Vertex);

    const ivec2 VertexMapPosition = ivec2(gl_GlobalInvocationID.xy);
    
    vec3 VertexPosition = vec3(VertexMapPosition, 0.0);
    VertexPosition.xy -= VertexMapSize / 2;
    VertexPosition.xy /= VertexMapSize / 2;
    VertexPosition.xy *= 5.0;

    vec3 CameraPosition = vec3(0.0, 0.0, 10.0);
    vec3 RayDirection = normalize(VertexPosition - CameraPosition);
    vec3 CurrentPosition = CameraPosition;

    imageStore(cs_Vertex, VertexMapPosition, vec4(GetStartLength(CameraPosition, RayDirection), GetEndLength(CameraPosition, RayDirection), 0.0, 1.0));
}

#endif // __INCLUDE_CS_KINECT_INTEGRATE_VOLUME_GLSL__