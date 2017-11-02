#ifndef __INCLUDE_CS_ESM_GLSL__
#define __INCLUDE_CS_ESM_GLSL__

#define EXPONENT 1.0f

layout (binding = 0) uniform sampler2D cs_InputImage;
layout (binding = 0, r32f) writeonly uniform image2D cs_OutputImage;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	int OutputX = int(gl_WorkGroupID.x);
	int OutputY = int(gl_WorkGroupID.y);
	
	int InputX = int(gl_GlobalInvocationID.x) * 8;
    int InputY = int(gl_GlobalInvocationID.y) * 8;
	
	vec2 UVNorm = vec2(InputX, InputY) / vec2(textureSize(cs_InputImage, 0));
	
	vec4 Accumulation = vec4(0.0f);
	
	Accumulation += textureGatherOffset(cs_InputImage, UVNorm, ivec2(0,0)) * EXPONENT;
	Accumulation += textureGatherOffset(cs_InputImage, UVNorm, ivec2(2,0)) * EXPONENT;
	Accumulation += textureGatherOffset(cs_InputImage, UVNorm, ivec2(0,2)) * EXPONENT;
	Accumulation += textureGatherOffset(cs_InputImage, UVNorm, ivec2(2,2)) * EXPONENT;
	
	float Depth  = dot(Accumulation, vec4(1.0f / 16.0f));
	
    imageStore(cs_OutputImage, ivec2(OutputX, OutputY), vec4(Depth));
}

#endif // __INCLUDE_CS_ESM_GLSL__