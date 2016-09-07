#ifndef __INCLUDE_FS_COPY_TO_GBUFFER_GLSL_
#define __INCLUDE_FS_COPY_TO_GBUFFER_GLSL_

#include "common_gbuffer.glsl"
#include "common_global.glsl"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
#define DEPTH_BIAS 0.00235

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
layout(binding = 0) uniform sampler2D ps_Texture0;
layout(binding = 1) uniform sampler2D ps_Texture1;
layout(binding = 2) uniform sampler2D ps_Texture2;
layout(binding = 3) uniform sampler2D ps_Texture3;

// -----------------------------------------------------------------------------
// Input from previous stage
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 fs_Texcoord;

// -----------------------------------------------------------------------------
// Output to fragment
// -----------------------------------------------------------------------------
layout(location = 0) out vec4 out_GBuffer0;
layout(location = 1) out vec4 out_GBuffer1;
layout(location = 2) out vec4 out_GBuffer2;

layout(depth_greater) out float gl_FragDepth;

// -----------------------------------------------------------------------------
// Function
// -----------------------------------------------------------------------------
vec3 ConvertDepthTo3D(in float _Depth, in vec2 _UV)
{
    float X;
    float Y;
    vec3 Position3D;

    X = _UV.x * 2.0f - 1.0f;
    Y = (1.0f - _UV.y) * 2.0f - 1.0f;
    
    X /= g_ViewToScreen[0][0];
    Y /= g_ViewToScreen[1][1];

    Position3D  = vec3(X, Y, -1.0f);
    Position3D  = normalize(Position3D);
    Position3D *= abs(_Depth);

    return Position3D;
}

// -----------------------------------------------------------------------------

float SampleDepthTexture(in sampler2D _DepthTexture, in vec2 _DepthUV)
{
    float Depth;

    // -----------------------------------------------------------------------------
    // P. Lensing originally used RGB char depth texture from Kinect 1:
    // float depth = v.r*31*2048 + v.g*63*32  + v.b*31;
    // -----------------------------------------------------------------------------
    Depth = textureLod( _DepthTexture, _DepthUV, 0 ).x;
    
    if (Depth <= 0)
    {
        return 10000;
    }

    return Depth;
}

// -----------------------------------------------------------------------------

vec3 GetVSNormal(in vec2 _DepthUV)
{
    vec2  DepthUVP;
    vec2  DepthUVN;
    float DepthInMetricUnitP;
    float DepthInMetricUnitN;
    float DifferenceP;
    float DifferenceN;
    vec3  DifferenceX1;
    vec3  DifferenceX2;
    vec3  DifferenceY1;
    vec3  DifferenceY2;

    // -----------------------------------------------------------------------------
    // Steps
    // -----------------------------------------------------------------------------
    float StepX = 1.0f / 1280.0f;
    float StepY = 1.0f / 720.0f;

    // -----------------------------------------------------------------------------
    // Depth at this fragment
    // -----------------------------------------------------------------------------
    float DepthInMetricUnit = SampleDepthTexture(ps_Texture1, _DepthUV);

    // -----------------------------------------------------------------------------
    // Depth in positive X
    // -----------------------------------------------------------------------------
    DepthUVP           = _DepthUV + vec2(StepX, 0.0f);
    DepthInMetricUnitP = SampleDepthTexture(ps_Texture1, DepthUVP);
    DifferenceP        = abs(DepthInMetricUnit - DepthInMetricUnitP);

    // -----------------------------------------------------------------------------
    // Depth in negative X
    // -----------------------------------------------------------------------------
    DepthUVN           = _DepthUV + vec2(-StepX, 0.0f);
    DepthInMetricUnitN = SampleDepthTexture(ps_Texture1, DepthUVN);
    DifferenceN        = abs(DepthInMetricUnit - DepthInMetricUnitN);

    // -----------------------------------------------------------------------------
    // Difference in X direction
    // -----------------------------------------------------------------------------
    if (DifferenceP < DifferenceN)
    {
        DifferenceX1 = ConvertDepthTo3D(DepthInMetricUnitP, DepthUVP);
        DifferenceX2 = ConvertDepthTo3D(DepthInMetricUnit , _DepthUV);
    }
    else
    {
        DifferenceX1 = ConvertDepthTo3D(DepthInMetricUnit , _DepthUV);
        DifferenceX2 = ConvertDepthTo3D(DepthInMetricUnitN, DepthUVN);
    }

    // -----------------------------------------------------------------------------
    // Depth in positive Y
    // -----------------------------------------------------------------------------
    DepthUVP           = _DepthUV + vec2(0.0f, StepY);
    DepthInMetricUnitP = SampleDepthTexture(ps_Texture1, DepthUVP);
    DifferenceP        = abs(DepthInMetricUnit - DepthInMetricUnitP);

    // -----------------------------------------------------------------------------
    // Depth in negative Y
    // -----------------------------------------------------------------------------
    DepthUVN           = _DepthUV + vec2(0.0f, -StepY);
    DepthInMetricUnitN = SampleDepthTexture(ps_Texture1, DepthUVN);
    DifferenceN        = abs(DepthInMetricUnit - DepthInMetricUnitN);

    // -----------------------------------------------------------------------------
    // Difference in Y direction
    // -----------------------------------------------------------------------------
    if (DifferenceP < DifferenceN)
    {
        DifferenceY1 = ConvertDepthTo3D(DepthInMetricUnitP, DepthUVP);
        DifferenceY2 = ConvertDepthTo3D(DepthInMetricUnit , _DepthUV);
    }
    else
    {
        DifferenceY1 = ConvertDepthTo3D(DepthInMetricUnit , _DepthUV);
        DifferenceY2 = ConvertDepthTo3D(DepthInMetricUnitN, DepthUVN);
    }

    // -----------------------------------------------------------------------------
    // Generate view-space normal from difference
    // -----------------------------------------------------------------------------    
    vec3 DifferenceX = DifferenceX2 - DifferenceX1;
    vec3 DifferenceY = DifferenceY2 - DifferenceY1;
    vec3 VSNormal    = vec3(normalize(cross(DifferenceY, DifferenceX)));

    return VSNormal;
}

// -----------------------------------------------------------------------------
// Blending: http://blog.selfshadow.com/publications/blending-in-detail/
// -----------------------------------------------------------------------------

vec3 blend_linear(in vec4 _Normal1, in vec4 _Normal2)
{
    vec3 r = ((_Normal1 + _Normal2)*2 - 2).xyz;
    
    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_pd(in vec4 _Normal1, in vec4 _Normal2)
{
    _Normal1 = _Normal1*2 - 1;
    _Normal2 = _Normal2.xyzz*vec4(2, 2, 2, 0) + vec4(-1, -1, -1, 0);
    vec3 r = _Normal1.xyz*_Normal2.z + _Normal2.xyw*_Normal1.z;

    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_whiteout(in vec4 _Normal1, in vec4 _Normal2)
{
    _Normal1 = _Normal1*2 - 1;
    _Normal2 = _Normal2*2 - 1;
    vec3 r = vec3(_Normal1.xy + _Normal2.xy, _Normal1.z*_Normal2.z);

    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_udn(in vec4 _Normal1, in vec4 _Normal2)
{
    vec3 c = vec3(2, 1, 0);
    vec3 r;
    r = _Normal2.xyz*c.yyz + _Normal1.xyz;
    r =  r*c.xxx -  c.xxy;

    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_rnm(in vec4 _Normal1, in vec4 _Normal2)
{
    vec3 t = _Normal1.xyz*vec3( 2,  2, 2) + vec3(-1, -1,  0);
    vec3 u = _Normal2.xyz*vec3(-2, -2, 2) + vec3( 1,  1, -1);
    vec3 r = t*dot(t, u) - u*t.z;

    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_unity(in vec4 _Normal1, in vec4 _Normal2)
{
    _Normal1 = _Normal1.xyzz*vec4(2, 2, 2, -2) + vec4(-1, -1, -1, 1);
    _Normal2 = _Normal2*2 - 1;
    
    vec3 r;
    r.x = dot(_Normal1.zxx,  _Normal2.xyz);
    r.y = dot(_Normal1.yzy,  _Normal2.xyz);
    r.z = dot(_Normal1.xyw, -_Normal2.xyz);
    
    return normalize(r);
}

// -----------------------------------------------------------------------------

vec3 blend_angle(in vec4 _Normal1, in vec4 _Normal2)
{
    float Angle = clamp(dot(_Normal1, _Normal2), 0.0f, 1.0f);
    
    vec3 ResultNormal;

    ResultNormal = mix(_Normal1.xyz, _Normal2.xyz, Angle);
    
    return normalize(ResultNormal);
}

// -----------------------------------------------------------------------------

void main()
{
    // -----------------------------------------------------------------------------
    // TexCoord for Kinect textures
    // -----------------------------------------------------------------------------
    vec2 TexCoord = vec2(fs_Texcoord.x, 1.0f - fs_Texcoord.y);

    // -----------------------------------------------------------------------------
    // Color
    // -----------------------------------------------------------------------------
    vec3  Color    = textureLod(ps_Texture0, TexCoord, 0).rgb;
    vec2  NormalXY = textureLod(ps_Texture2, TexCoord, 0).rg;
    float NormalZ  = textureLod(ps_Texture3, TexCoord, 0).a;

    vec4 GBufferWSNormal = vec4(NormalXY, NormalZ, 0);
    // vec4 GBufferWSNormal = vec4(0, 0, 1, 0);

    // -----------------------------------------------------------------------------
    // Normal
    // -----------------------------------------------------------------------------
    vec3 VSNormal = GetVSNormal(TexCoord);
    vec4 WSNormal = g_ViewToWorld * vec4(VSNormal, 0.0f);

    WSNormal.xyz = blend_angle(WSNormal, GBufferWSNormal);

    // -----------------------------------------------------------------------------
    // Pack to G-Buffer
    // -----------------------------------------------------------------------------
    SGBuffer GBuffer;
    
    PackGBuffer(Color, WSNormal.xyz, 1.0f, vec3(0.0f), 0.0f, 1.0f, GBuffer);

    out_GBuffer0 = GBuffer.m_Color0;
    out_GBuffer1 = GBuffer.m_Color1;
    out_GBuffer2 = GBuffer.m_Color2;
    
    // -----------------------------------------------------------------------------
    // Writing depth to depth buffer manually
    // -----------------------------------------------------------------------------
    float Depth  = SampleDepthTexture(ps_Texture1, TexCoord);

    Depth = Depth / 0.003f;

    gl_FragDepth = ConvertToHyperbolicDepth(Depth, g_ViewToScreen);
}

#endif // __INCLUDE_FS_COPY_TO_GBUFFER_GLSL_