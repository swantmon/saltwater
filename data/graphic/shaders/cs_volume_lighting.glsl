#ifndef __INCLUDE_CS_VOLUME_LIGHTING_GLSL__
#define __INCLUDE_CS_VOLUME_LIGHTING_GLSL__

layout (binding = 0, r32f) readonly  uniform image2D cs_InputImage;
layout (binding = 1, rgba16f) writeonly uniform image3D cs_OutputImage;

layout (binding = 2, rgba16f) readonly uniform image2D cs_PermutationImage;
layout (binding = 3, rgba16f) readonly uniform image2D cs_PermutationGradientImage;

// -------------------------------------------------------------------------------------
// Simon Green. 
// “Implementing Improved Perlin Noise.” 
// In GPU Gems 2, edited by Matt Farr, pp. 409–416, Addison-Wesley Professional, 2005.
// -------------------------------------------------------------------------------------
vec3 GetFade(in vec3 t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); // new curve
//	return t * t * (3 - 2 * t); // old curve
}

vec4 SamplePermutation(in vec2 _UV)
{
	return imageLoad(cs_PermutationImage, ivec2(_UV * vec2(256.0f)));
}

float SampleGradientPermutation(in float _U, in vec3 _Point)
{
	return dot(imageLoad(cs_PermutationImage, ivec2(_U * 256, 0)).xyz, _Point);
}

float ImprovedPerlinNoise3D(in vec3 _Seed)
{
	// -------------------------------------------------------------------------------------
	// 1. FIND UNIT CUBE THAT CONTAINS POINT
	// 2. FIND RELATIVE X,Y,Z OF POINT IN CUBE.
	// 3. COMPUTE FADE CURVES FOR EACH OF X,Y,Z.
	// -------------------------------------------------------------------------------------
	vec3 UV = mod(floor(_Seed), vec3(256.0f));
  	_Seed -= floor(_Seed);

	vec3 Fade = GetFade(_Seed);

	UV = UV / 256.0f;

	const float One = 1.0f / 256.0f;
	
	// -------------------------------------------------------------------------------------
    // HASH COORDINATES OF THE 8 CUBE CORNERS
    // -------------------------------------------------------------------------------------
	vec4 Hash = SamplePermutation(UV.xy) + UV.z;
 
 	// -------------------------------------------------------------------------------------
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
	// -------------------------------------------------------------------------------------
  	return mix( mix( mix( SampleGradientPermutation(Hash.x, _Seed ),  
                          SampleGradientPermutation(Hash.z, _Seed + vec3(-1.0f,  0.0f, 0.0f) ), Fade.x),
                     mix( SampleGradientPermutation(Hash.y, _Seed + vec3( 0.0f, -1.0f, 0.0f) ),
                          SampleGradientPermutation(Hash.w, _Seed + vec3(-1.0f, -1.0f, 0.0f) ), Fade.x), Fade.y),
                           
                mix( mix( SampleGradientPermutation(Hash.x + One, _Seed + vec3( 0.0f,  0.0f, -1.0f) ),
                          SampleGradientPermutation(Hash.z + One, _Seed + vec3(-1.0f,  0.0f, -1.0f) ), Fade.x),
                     mix( SampleGradientPermutation(Hash.y + One, _Seed + vec3( 0.0f, -1.0f, -1.0f) ),
                          SampleGradientPermutation(Hash.w + One, _Seed + vec3(-1.0f, -1.0f, -1.0f) ), Fade.x), Fade.y), Fade.z);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint X = gl_GlobalInvocationID.x;
    uint Y = gl_GlobalInvocationID.y;
    uint Z = gl_GlobalInvocationID.z;

    vec4 Pixel = imageLoad(cs_InputImage, ivec2(X, Y));

    float PerlinNoise = ImprovedPerlinNoise3D(vec3(X / 160.0f, Y / 90.0f, Z / 128.0f));

    imageStore(cs_OutputImage, ivec3(X, Y, Z), vec4(PerlinNoise * 100.0f, 0, 0, 0));
}

#endif // __INCLUDE_CS_VOLUME_LIGHTING_GLSL__