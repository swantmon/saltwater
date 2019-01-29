
#ifndef __INCLUDE_FS_OUTLINE_GLSL__
#define __INCLUDE_FS_OUTLINE_GLSL__

layout (binding = 0, MAP_TEXTURE_FORMAT) uniform image2D fs_VertexMap;
layout (binding = 1, MAP_TEXTURE_FORMAT) uniform image2D fs_NormalMap;

layout(location = 0) in vec2 in_TexCoords;

layout(location = 0) out vec4 out_Color;

void main()
{
    ivec2 Coords = ivec2(gl_FragCoord.x, DEPTH_IMAGE_HEIGHT - gl_FragCoord.y);

    vec3 Vertex = imageLoad(fs_VertexMap, Coords).xyz;
    vec3 Normal = imageLoad(fs_NormalMap, Coords).xyz;

    vec3 Direction = normalize(vec3(0.0f) - Vertex);

    float Light = max(0.0f, dot(normalize(Normal), Direction));

    out_Color = vec4(Light, Light, Light, 1.0f);
}

#endif // __INCLUDE_FS_OUTLINE_GLSL__