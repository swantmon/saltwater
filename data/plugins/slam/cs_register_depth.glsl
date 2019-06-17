#ifndef __INCLUDE_CS_REGISTER_DEPTH_GLSL__
#define __INCLUDE_CS_REGISTER_DEPTH_GLSL__

layout(binding = 0, r32ui) coherent uniform uimage2D cs_RegisteredDepth;
layout(binding = 1, r16ui) readonly uniform uimage2D cs_UnregisteredDepth;

layout(std140, binding = 0) uniform UBOIntrinsics
{
    vec2 g_ColorFocalLength;
    vec2 g_ColorFocalPoint;
    vec2 g_DepthFocalLength;
    vec2 g_DepthFocalPoint;
    mat4 g_DepthToCameraTransform;
    mat4 g_CameraToDepthTransform;
};

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    const ivec2 ImageSize = imageSize(cs_UnregisteredDepth);

    ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);

    const float Depth = imageLoad(cs_UnregisteredDepth, Coords).x / 1000.0f;

    vec3 Vertex;    
    Vertex.xy = Depth * (Coords - g_DepthFocalPoint) / g_DepthFocalLength;
    Vertex.z = Depth;

    if (Depth > 0.0f)
    {
        Vertex = (g_DepthToCameraTransform * vec4(Vertex, 1.0f)).xyz;

        ivec2 ColorCoords = ivec2((Vertex.xy * g_ColorFocalLength) / Vertex.z + g_ColorFocalPoint);
        uint RegisteredDepth = uint(Vertex.z * 1000.0f);

        imageAtomicMin(cs_RegisteredDepth, ColorCoords, RegisteredDepth);
    }
}

#endif //__INCLUDE_CS_REGISTER_DEPTH_GLSL__