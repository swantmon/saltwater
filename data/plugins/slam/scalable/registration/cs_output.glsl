#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    mat2 Transform = mat2(g_A, g_B, -g_B, g_A);

    vec2 FixedCoords = vec2(gl_GlobalInvocationID.xy) / g_FixedImageSize;
    vec2 MovingCoords = inverse(Transform) * (FixedCoords - g_Translation);

    float FixedColor = RGBToGrey(texture(FixedTex, FixedCoords).rgb);
    float MovingColor = 0.0f;

    if (MovingCoords.x > 0.001f && MovingCoords.x < 0.99f && MovingCoords.y > 0.001f && MovingCoords.y < 0.99f)
    {
        MovingColor = RGBToGrey(texture(MovingTex, MovingCoords).rgb);
    }

    ivec2 OutputCoords = ivec2(gl_GlobalInvocationID.xy);
    OutputCoords.y = g_FixedImageSize.y - OutputCoords.y;
    imageStore(OutputImage, ivec2(OutputCoords), vec4(FixedColor, MovingColor, 0.0f, 1.0f));
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__