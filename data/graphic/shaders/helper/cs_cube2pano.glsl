#ifndef __INCLUDE_CS_CUBE2PANO_GLSL__
#define __INCLUDE_CS_CUBE2PANO_GLSL__

// #define TILE_SIZE 8
// #define CUBE_TYPE rgba8
// #define OUTPUT_TYPE rgba8
// #define CUBE_SIZE 512
#define BIAS 0.99

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, CUBE_TYPE) readonly uniform imageCube in_Cubemap;
layout (binding = 1, OUTPUT_TYPE) writeonly uniform image2D out_Panorama;

// -------------------------------------------------------------------------------------

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    uint X;
    uint Y;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;

    // -----------------------------------------------------------------------------
    // Polar coordindates
    // Note: Within a pixel shader we could simply use the direction vector to 
    // sample the cube map.
    // -----------------------------------------------------------------------------
    vec3 UnitDirection = vec3(0.0f, 0.0f, 0.0f);

    float Theta = float(Y) / 64.0f * 3.141592653589793f;
    float Phi   = float(X) / 128.0f * 2.0f * 3.141592653589793f;

    UnitDirection.x = sin(Phi) * sin(Theta) * - 1.0f;
    UnitDirection.y = cos(Theta);
    UnitDirection.z = cos(Phi) * sin(Theta) * - 1.0f;

    // -----------------------------------------------------------------------------
    // Cube map coordinates
    // -----------------------------------------------------------------------------
    vec3 UnitUV = vec3(0.0f, 0.0f, 0.0f);

    float a = max(max(abs(UnitDirection.x), abs(UnitDirection.y)), abs(UnitDirection.z));

    UnitUV.x = UnitDirection.x / a;
    UnitUV.y = UnitDirection.y / a;
    UnitUV.z = UnitDirection.z / a;

    ivec3 UV = ivec3(0, 0, -1);

    if (UnitUV.x >= BIAS) //Right
    {
        UV.x = int((((UnitUV.z + 1.0f) / 2.0f) - 1.0f) * CUBE_SIZE);
        UV.y = int((((UnitUV.y + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.z = 0;
    }
    else if (UnitUV.x <= -BIAS) // Left
    {
        UV.x = int((((UnitUV.z + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.y = int((((UnitUV.y + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.z = 1;
    }
    else if (UnitUV.y >= BIAS) // Up
    {
        UV.x = int((((UnitUV.x + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.y = int((((UnitUV.z + 1.0f) / 2.0f) - 1.0f) * CUBE_SIZE);
        UV.z = 2;
    }
    else if (UnitUV.y <= -BIAS) // Down
    {
        UV.x = int((((UnitUV.x + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.y = int((((UnitUV.z + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.z = 3;
    }
    else if (UnitUV.z >= BIAS) // Front
    {
        UV.x = int((((UnitUV.x + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.y = int((((UnitUV.y + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.z = 4;
    }
    else if (UnitUV.z <= -BIAS) // Back
    {
        UV.x = int((((UnitUV.x + 1.0f) / 2.0f) - 1.0f) * CUBE_SIZE);
        UV.y = int((((UnitUV.y + 1.0f) / 2.0f)) * CUBE_SIZE);
        UV.z = 5;
    }

    // -----------------------------------------------------------------------------
    // Read pixel and save to panorama
    // -----------------------------------------------------------------------------
    UV.x = abs(UV.x);
    UV.y = abs(UV.y);

    vec4 Texel = imageLoad(in_Cubemap, UV);

    imageStore(out_Panorama, ivec2(X, Y), Texel);
}

#endif // __INCLUDE_CS_CUBE2PANO_GLSL__