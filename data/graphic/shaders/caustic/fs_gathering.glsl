#ifndef __INCLUDE_FS_GATHERING_GLSL__
#define __INCLUDE_FS_GATHERING_GLSL__

// -----------------------------------------------------------------------------
// Original approach:
// "Interactive image-space techniques for approximating caustics"
// Chris Wyman, Scott Davis (2006)
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#ifndef CAUSTIC_MAP_RESOLUTION 
    #define CAUSTIC_MAP_RESOLUTION 1024.0f
#endif

#ifndef PHOTON_RESOLUTION_MULTIPLIER 
    #define PHOTON_RESOLUTION_MULTIPLIER 0.001f
#endif

// -----------------------------------------------------------------------------
// Input from prev. stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec4 in_NormalizedCoords;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
void main(void)
{
    float SizeOfSplat      = 2.5f;
    float SizeOfSplatSqr   = SizeOfSplat * SizeOfSplat;
    float IsInsideGaussian = 0.0f;
 
    // -----------------------------------------------------------------------------
    // We need to compute how far this fragment is from the center of the point.  
    // We could do this using point sprites, but the final framerate is 10x 
    // faster this way.
    // -----------------------------------------------------------------------------
    vec2 FragmentLocation = gl_FragCoord.xy;
    vec2 PointLocation    = (in_NormalizedCoords.xy * 0.5f + 0.5f) * vec2(CAUSTIC_MAP_RESOLUTION);
    
    // -----------------------------------------------------------------------------
    // Gaussian from Graphics Gems I, "Convenient anti-aliasing filters that 
    // minimize bumpy sampling"
    // -----------------------------------------------------------------------------
    float Alpha       = 0.918f;
    float Beta2       = 3.906f;       // == beta*2 == 1.953 * 2; 
    float Denominator = 0.858152111f; // == 1 - exp(-beta);
    
    float DistanceSqrToSplatCtr = dot(FragmentLocation - PointLocation, FragmentLocation - PointLocation);
    float ResultExp             = exp(-Beta2 * DistanceSqrToSplatCtr / SizeOfSplatSqr);
    
    IsInsideGaussian = DistanceSqrToSplatCtr/SizeOfSplatSqr < 0.25f ? 1.0f : 0.0f;

    float NormalizeFactor = 10.5f * SizeOfSplatSqr / 25.0f;
    
    ResultExp = Alpha + Alpha * ((ResultExp - 1.0f) / Denominator);

    out_Output = vec4(PHOTON_RESOLUTION_MULTIPLIER * IsInsideGaussian * ResultExp / NormalizeFactor);
}

#endif // __INCLUDE_FS_GATHERING_GLSL__