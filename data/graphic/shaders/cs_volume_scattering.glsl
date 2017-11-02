#ifndef __INCLUDE_CS_VOLUME_SCATTERING_GLSL__
#define __INCLUDE_CS_VOLUME_SCATTERING_GLSL__

layout (binding = 0, rgba32f) readonly uniform image3D cs_InputImage;
layout (binding = 1, rgba32f) writeonly uniform image3D cs_OutputImage;

// -------------------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------------------
vec4 AccumulateScattering(in vec4 _ColorAndDensityFront, in vec4 _ColorAndDensityBack)
{
	vec3 Light = _ColorAndDensityFront.rgb + clamp(exp(-_ColorAndDensityFront.a), 0.0f, 1.0f) * _ColorAndDensityBack.rgb;
	
	return vec4(Light.rgb, _ColorAndDensityFront.a + _ColorAndDensityBack.a);
}

vec4 ReadScattering(in ivec3 _UV)
{
	return imageLoad(cs_InputImage, _UV);
}

void WriteScattering(in ivec3 _UV, in vec4 _ColorAndDensity)
{
	vec4 FinalOutput = vec4(_ColorAndDensity.rgb, exp(-_ColorAndDensity.a));
	
	imageStore(cs_OutputImage, _UV, FinalOutput);
}

layout(local_size_x = 16, local_size_y = 10, local_size_z = 1) in;
void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;

	// -------------------------------------------------------------------------------------
	// First slice
	// -------------------------------------------------------------------------------------
    vec4 CurrentSlice = imageLoad(cs_InputImage, ivec3(X, Y, 0));
	
	WriteScattering(ivec3(X, Y, 0), CurrentSlice);
	
	for (int SliceZ = 1; SliceZ < 128; ++ SliceZ)
	{
		vec4 NextSlice = ReadScattering(ivec3(X, Y, SliceZ));
		
		CurrentSlice = AccumulateScattering(CurrentSlice, NextSlice);
		
		WriteScattering(ivec3(X, Y, SliceZ), CurrentSlice);
	}
}

#endif // __INCLUDE_CS_VOLUME_SCATTERING_GLSL__