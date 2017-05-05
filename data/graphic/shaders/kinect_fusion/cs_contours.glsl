
#ifndef __INCLUDE_CS_CONTOURS_GLSL__
#define __INCLUDE_CS_CONTOURS_GLSL__

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------

layout (binding = 0, r16ui) readonly uniform uimage2D cs_DepthBuffer;
layout (binding = 1, rgba32f) writeonly uniform image2D cs_Contours;

// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE2D, local_size_y = TILE_SIZE2D, local_size_z = 1) in;
void main()
{
    const int x = int(gl_GlobalInvocationID.x);
    const int y = int(gl_GlobalInvocationID.y);

    const ivec2 ImageSize = imageSize(cs_DepthBuffer);

    if (x > 0 && x < ImageSize.x - 1 && y > 0 && y < ImageSize.y - 1)
    {
        ivec2 Coords[8];

        Coords[0] = ivec2(x + 1, y    );
        Coords[1] = ivec2(x + 1, y + 1);
        Coords[2] = ivec2(x    , y + 1);
        Coords[3] = ivec2(x - 1, y + 1);
        Coords[4] = ivec2(x - 1, y    );
        Coords[5] = ivec2(x - 1, y - 1);
        Coords[6] = ivec2(x    , y - 1);
        Coords[7] = ivec2(x + 1, y - 1);

        const int Center = int(imageLoad(cs_DepthBuffer, ivec2(x, y)).x);

        for (int i = 0; i < 8; ++ i)
        {
            const int Sample = int(imageLoad(cs_DepthBuffer, Coords[i]).x);

            if (Center - Sample > 0.05f * 1000.0f) // 0.05 Meters
            {
                imageStore(cs_Contours, ivec2(x, y), vec4(1.0f));
                return;
            }
        }
    }
    imageStore(cs_Contours, ivec2(x, y), vec4(0.0f));
}

#endif // __INCLUDE_CS_CONTOURS_GLSL__