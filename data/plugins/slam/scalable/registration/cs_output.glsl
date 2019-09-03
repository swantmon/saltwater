#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    mat2 Transform = mat2(g_A, g_B, -g_B, g_A);

    vec2 MovingCoords = vec2(gl_GlobalInvocationID.xy);
    vec2 FixedCoords = Transform * MovingCoords + g_Translation;

    if (FixedCoords.x >= 0.0f && FixedCoords.x < g_FixedImageSize.x && FixedCoords.y >= 0.0f && FixedCoords.y < g_FixedImageSize.y)
    {
        float MovingColor = RGBToGrey(texture(MovingTex, MovingCoords / g_MovingImageSize).rgb);
        float FixedColor = RGBToGrey(texture(FixedTex, FixedCoords / g_FixedImageSize).rgb);

        MovingCoords.y = imageSize(MovingImage).y - MovingCoords.y;
        imageStore(OutputImage, ivec2(MovingCoords), vec4(MovingColor, FixedColor, 0.0f, 1.0f));
    }
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__