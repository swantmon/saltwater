#ifndef __INCLUDE_CS_YUV_TO_RGB_GLSL__
#define __INCLUDE_CS_YUV_TO_RGB_GLSL__

layout (binding = 0, r8) readonly uniform image2D cs_Y;
layout (binding = 1, rg8) readonly uniform image2D cs_UV;
layout (binding = 2, rgba8) writeonly uniform image2D cs_RGB;

layout (local_size_x = TILE_SIZE_2D, local_size_y = TILE_SIZE_2D, local_size_z = 1) in;
void main()
{
    const ivec2 Coords = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 ImageSize = imageSize(cs_RGB);
    if (Coords.x < ImageSize.x && Coords.y < ImageSize.y)
    {
        // https://developer.apple.com/documentation/arkit/arframe/2867984-capturedimage
        mat4 YCbCrToRGBTransform = mat4(
            vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
            vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
            vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
            vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
        );
        
        float YColor = imageLoad(cs_Y, Coords).x;
        vec2 UVColor = imageLoad(cs_UV, Coords / 2).xy;

        vec4 RGBColor = YCbCrToRGBTransform * vec4(YColor, UVColor, 1.0f);

        imageStore(cs_RGB, Coords, vec4(RGBColor));
    }
}

#endif //__INCLUDE_CS_YUV_TO_RGB_GLSL__