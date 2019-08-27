#ifndef __INCLUDE_CS_SUM_TILES_GLSL__
#define __INCLUDE_CS_SUM_TILES_GLSL__

#include "../../plugins/slam/scalable/registration/common.glsl"

shared vec4 g_SharedData[TILE_SIZE2D * TILE_SIZE2D];

void reduce()
{
    for (int i = TILE_SIZE2D * TILE_SIZE2D; i >= 1; i /= 2)
    {
        if (gl_LocalInvocationIndex < i / 2)
        {
            g_SharedData[gl_LocalInvocationIndex] += g_SharedData[gl_LocalInvocationIndex + i / 2];
        }
        barrier();
    }
}

// Every thread relates to a pixel in the moving image (just a convention)

// Evaluated per pixel and then summed:
// -2 * (Fixed(p) - Moving(T(p;theta))) * Gradient * dT/dtheta

// T(p; theta): ax - by + tx
// T(p; theta): bx + ay + ty
// theta = (a b tx ty)
// p = (x y)
// dT/dtheta: x -y 1 0
// dT/dtheta: y  x 0 1

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    vec2 MovingCoords = vec2(gl_GlobalInvocationID.xy) / g_MovingImageSize;
    vec2 FixedCoords = (mat3(g_Transform) * vec3(MovingCoords, 1.0f)).xy;

    float MovingColor = RGBToGrey(texture(MovingTex, MovingCoords).rgb);
    float FixedColor = RGBToGrey(texture(FixedTex, FixedCoords).rgb);
    vec2 Gradient = texture(GradientTex, MovingCoords).rg;

    float IntensityDiff = FixedColor - MovingColor; // Fixed(p) - Moving(T(p;theta))

    vec2 Factor = -2.0f * IntensityDiff * Gradient; // -2 * (Fixed(p) - Moving(T(p;theta))) * Gradient

    //g_SharedData[gl_LocalInvocationIndex] = ;
    barrier();

    reduce();

    if (gl_LocalInvocationIndex == 0)
    {
        g_Sum[gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_WorkGroupID.y] = g_SharedData[0];
    }
}

#endif //__INCLUDE_CS_SUM_TILES_GLSL__