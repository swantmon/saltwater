#ifndef __INCLUDE_CS_PANO2CUBE_GLSL__
#define __INCLUDE_CS_PANO2CUBE_GLSL__

// #define TILE_SIZE 1
// #define CUBE_TYPE rgba8
// #define OUTPUT_TYPE rgba8
// #define CUBE_SIZE 512
// #define PANORAMA_SIZE_W 128
// #define PANORAMA_SIZE_H 64
#define PI 3.14159265359f

// -----------------------------------------------------------------------------
// Output
// -----------------------------------------------------------------------------
layout (binding = 0, OUTPUT_TYPE) readonly uniform image2D in_Panorama;
layout (binding = 1, CUBE_TYPE) writeonly uniform imageCube out_Cubemap;

// -------------------------------------------------------------------------------------

vec4 GetTextureSpherical(in vec3 _Normal)
{
    float Phi   = acos(_Normal.y);
    float Theta = atan(-1.0f * _Normal.x, _Normal.z) + PI;

    vec2 UV = vec2(Theta / (2.0f * PI), 1.0f - Phi / PI);

    UV.x *= PANORAMA_SIZE_W;
    UV.y *= PANORAMA_SIZE_H;

    return imageLoad(in_Panorama, ivec2(UV));
}

vec3 GetNormal(in uint _Face, in float _u, in float _v)
{
    vec3 Normal = vec3(0.0, 0.0, 0.0);

    if (_Face == 0)
        Normal = vec3( -_u,  -_v,  1.0);           // ZP
    else if (_Face == 1)
        Normal = vec3(  _u,  -_v, -1.0);           // ZM
    else if (_Face == 2)
        Normal = vec3(  _v,  1.0,  _u);            // YP
    else if (_Face == 3)
        Normal = vec3( -_v, -1.0,   _u);           // YM
    else if (_Face == 4)
        Normal = vec3( 1.0,  -_v,   _u);           // XP
    else if (_Face == 5)
        Normal = vec3(-1.0,  -_v,  -_u);           // XM

    return normalize(Normal);
}

layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    uint X;
    uint Y;
    uint F;

    X = gl_GlobalInvocationID.x;
    Y = gl_GlobalInvocationID.y;
    F = gl_GlobalInvocationID.z;

    vec2 UnitUV = vec2((float(X) / float(CUBE_SIZE)) * 2.0f - 1.0f, (float(Y) / float(CUBE_SIZE)) * 2.0f - 1.0f);

    vec3 Normal = GetNormal(F, UnitUV.x, UnitUV.y);

    vec4 Texel = GetTextureSpherical(Normal);

    imageStore(out_Cubemap, ivec3(X, Y, F), Texel);
}

#endif // __INCLUDE_CS_PANO2CUBE_GLSL__