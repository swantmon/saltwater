
#ifndef __INCLUDE_TES_TEST_GLSL__
#define __INCLUDE_TES_TEST_GLSL__

#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Built-in variables
// -----------------------------------------------------------------------------
in gl_PerVertex
{
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex
{
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

// -----------------------------------------------------------------------------
// Layout
// -----------------------------------------------------------------------------
layout(triangles, equal_spacing, ccw) in;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 6) uniform sampler2D tes_TextureBump;

float gDispFactor = 0.1f;

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_Position[];
layout(location = 1) in vec3 in_Normal[];
layout(location = 2) in vec2 in_TexCoord[];
layout(location = 3) in mat3 in_WSNormalMatrix[];

// -----------------------------------------------------------------------------
// Output to next stage
// -----------------------------------------------------------------------------
layout(location = 0) out vec3 out_Position;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out vec2 out_TexCoord;
layout(location = 3) out mat3 out_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
   	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

// -----------------------------------------------------------------------------

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
   	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

// -----------------------------------------------------------------------------

void main()
{
    // -----------------------------------------------------------------------------
    // Interpolate the attributes of the output vertex using the barycentric
    // coordinates
    // -----------------------------------------------------------------------------
   	out_TexCoord = interpolate2D(in_TexCoord[0], in_TexCoord[1], in_TexCoord[2]);
   	out_Normal   = interpolate3D(in_Normal[0], in_Normal[1], in_Normal[2]);
   	out_Normal   = normalize(out_Normal);
   	out_Position = interpolate3D(in_Position[0], in_Position[1], in_Position[2]);
    out_WSNormalMatrix = in_WSNormalMatrix[0];
    
    // TODO by tschwandt (13.08.2015)
    // Normal matrix has to be computed here in this shader
    // -> therefore we have to change automatic shader loading in
    //    model and material manager

    // -----------------------------------------------------------------------------
    // Displace the vertex along the normal
    // -----------------------------------------------------------------------------
   	float Displacement = texture(tes_TextureBump, out_TexCoord.xy).x;

   	out_Position += (out_Normal * Displacement * gDispFactor);

   	gl_Position = ps_WorldToScreen * vec4(out_Position, 1.0f);
}

#endif // __INCLUDE_TES_TEST_GLSL__