#ifndef __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__
#define __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(std140, binding = 3) uniform UB3
{
    mat4 ps_LightProjectionMatrix;
    mat4 ps_LightViewMatrix;
    uint ps_ExposureHistoryIndex;
};

layout(binding = 0) uniform sampler2D ps_RefractiveNormal;
layout(binding = 1) uniform sampler2D ps_RefractiveDepth;
layout(binding = 2) uniform sampler2D ps_BackgroundDepth;

// -----------------------------------------------------------------------------
// Input to fragment from previous stage
// -----------------------------------------------------------------------------
layout(location = 0) in vec3 in_WSPosition;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat3 in_WSNormalMatrix;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_PhotonLocation;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------


vec2 ProjectToTexCoord( vec4 eyeSpacePos )
{
    vec4 projLoc = ps_LightProjectionMatrix * eyeSpacePos;
    return ( 0.5*(projLoc.xy / projLoc.w) + 0.5 );
}

vec4 refraction( vec3 incident, vec3 normal, float ni_nt, float ni_nt_sqr )
{
    vec4 returnVal;
    float IdotN = dot( -incident, normal );
    float cosSqr = 1.0 - ni_nt_sqr*(1.0 - IdotN*IdotN);
    return ( cosSqr <= 0.0 ? 
             vec4( reflect( incident, normal ).xyz, -1 ) : 
             vec4( normalize( ni_nt * incident + (ni_nt * IdotN - sqrt( cosSqr )) * normal ).xyz, 1) 
           ); 
}

void main(void)
{
    vec3 WSNormal = in_Normal;
    vec3 VSNormal = (ps_LightViewMatrix * vec4(WSNormal, 0.0f)).xyz;

    vec3 SSPosition;

    SSPosition.xy = gl_FragCoord.xy / vec2(1024.0f);
    SSPosition.z  = gl_FragCoord.z;

    vec3 WSPosition = in_WSPosition;
    vec3 VSPosition = (ps_LightViewMatrix * vec4(WSPosition, 1.0f)).xyz;
 

float near = 0.1f;
float far  = 20.1f;
float indexOfRefraction = 1.41f;
vec4 local1 = vec4(near*far, far-near, far+near, far);
vec4 local2 = vec4(1.0/indexOfRefraction, 1.0/(indexOfRefraction*indexOfRefraction), indexOfRefraction, indexOfRefraction*indexOfRefraction);







    // Stuff that we know from the beginning
    vec3 N_1 = normalize( VSNormal );   // Surface Normal
    vec3 V   = normalize( VSPosition  );   // View direction
    
    // Find the relective (.x) and refractive (.y) fresnel coefficients 
    //vec2 fresnel = fresnelApprox( dot( -V, N_1 ) );

    // Find the distance to front & back surface, first as normalized [0..1] values, than unprojected
    vec2 Dist = vec2( texture(ps_RefractiveDepth, SSPosition.xy).x, SSPosition.z );
    Dist = local1.x / (Dist * local1.y - local1.z );
    
    // find the refraction direction
    vec3 T_1 = refraction( V, N_1, local2.x, local2.y ).xyz; 

    float d_V = Dist.y - Dist.x;

    // Compute approximate exitant location & surface normal
    //vec4 P_2_tilde = vec4( T_1 * d_V * multiplier + gl_TexCoord[2].xyz, 1.0);
    vec4 P_2_tilde = vec4( T_1 * d_V + VSPosition.xyz, 1.0);
    vec3 N_2 = texture2D( ps_RefractiveNormal, ProjectToTexCoord( P_2_tilde ) ).xyz;

    N_2 = (ps_LightViewMatrix * vec4(N_2, 0.0f)).xyz;

    float dotN2 = dot( N_2.xyz, N_2.xyz );
    N_2 = normalize( N_2 );

    // What happens if we lie in a black-texel?  Means no normal!  Conceptually,
    //   this means we pass thru "side" of object.  Use norm perpindicular to view
    if ( dotN2 == 0.0 )
        N_2 = normalize( vec3( T_1.x, T_1.y, 0 ) );

    // Refract at the second surface
    vec4 T_2 = refraction( T_1, -N_2, local2.z, local2.w );
    float TotalInternalReflectionTIR = T_2.w;
    
    // Scale the vector so that it's got a unit-length z-component
    vec4 scaled_T_2 = vec4(T_2.xyz,0) / -T_2.z;

    // Compute the texture locations of ctrPlusT2 and refractToNear.
    float index, minDist = 1000.0, deltaDist = 1000.0;
    for (index = 0.0; index < 2.0; index += 1.0)
    {
        float texel = texture2D( ps_BackgroundDepth, ProjectToTexCoord( P_2_tilde + scaled_T_2 * index ) ).x;
        float distA = -(local1.x / (texel * local1.y - local1.w)) + P_2_tilde.z;
        if ( abs(distA-index) < deltaDist )
        {
            deltaDist = abs(distA-index);
            minDist = index;
        }
    }
    
    float distOld = minDist;    
    for (index = 0.0; index < 10.0; index += 1.0)
    {
        float texel1 = texture2D( ps_BackgroundDepth, ProjectToTexCoord( P_2_tilde + distOld * scaled_T_2 ) ).x;
        distOld = -(local1.x / (texel1 * local1.y - local1.w)) + P_2_tilde.z;
    }

    out_PhotonLocation.xyz = P_2_tilde.xyz + distOld * scaled_T_2.xyz;
    out_PhotonLocation.w   = TotalInternalReflectionTIR;








    //out_PhotonLocation.xyz = vec3(d_V);
    //out_PhotonLocation.xyz = vec3(Dist, 1.0f);
    //out_PhotonLocation.xyz = vec3(vec2( texture( ps_RefractiveDepth, SSPosition.xy ).r, SSPosition.z ), 1.0f);
    //out_PhotonLocation.xyz = SSPosition;
    //out_PhotonLocation = gl_FragCoord;
}

#endif // __INCLUDE_FS_CAUSTIC_EMISSION_GLSL__