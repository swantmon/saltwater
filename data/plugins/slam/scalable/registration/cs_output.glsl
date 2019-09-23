#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    mat2 Transform = mat2(g_A, g_B, -g_B, g_A);

    vec2 FixedCoords = vec2(gl_GlobalInvocationID.xy);
    vec2 MovingCoords = inverse(Transform) * (FixedCoords - g_Translation);

    float FixedColor = RGBToGrey(texture(FixedTex, FixedCoords / g_FixedImageSize).rgb);
    float MovingColor = 0.0f;

    if (MovingCoords.x > 0.0f && MovingCoords.x < g_MovingImageSize.x && MovingCoords.y > 0.0f && MovingCoords.y < g_MovingImageSize.y)
    {
        MovingColor = RGBToGrey(texture(MovingTex, MovingCoords / g_MovingImageSize).rgb);
    }
    
    FixedCoords.y = g_FixedImageSize.y - FixedCoords.y;
    imageStore(OutputImage, ivec2(FixedCoords), vec4(FixedColor, MovingColor, 0.0f, 1.0f));
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__