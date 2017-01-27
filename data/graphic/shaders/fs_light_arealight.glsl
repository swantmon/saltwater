
#ifndef __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__
#define __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__

#include "common_global.glsl"
#include "common.glsl"
#include "common_light.glsl"
#include "common_gbuffer.glsl"

// -----------------------------------------------------------------------------
// Input from system
// -----------------------------------------------------------------------------
in vec4 gl_FragCoord;

// -----------------------------------------------------------------------------
// Input from engine
// -----------------------------------------------------------------------------
const float LUT_SIZE  = 64.0;
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;

const float pi = 3.14159265;

layout(row_major, std140, binding = 1) uniform UB1
{
    mat4  view;
    vec4  dcolor;
    vec4  scolor;
    vec2  resolution;
    float intensity;
    float width;
    float height;
    float roty;
    float rotz;
    float padding;
    bool  twoSided;
    int   sampleCount;
    uint  ps_ExposureHistoryIndex;
};

layout(std430, binding = 0) buffer UExposureHistoryBuffer
{
    float ps_ExposureHistory[8];
};

layout(binding = 0) uniform sampler2D ps_GBuffer0;
layout(binding = 1) uniform sampler2D ps_GBuffer1;
layout(binding = 2) uniform sampler2D ps_GBuffer2;
layout(binding = 3) uniform sampler2D ps_DepthTexture;
layout(binding = 4) uniform sampler2D ltc_mat;
layout(binding = 5) uniform sampler2D ltc_mag;

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
layout(location = 2) in vec2 in_TexCoord;

// -----------------------------------------------------------------------------
// Output to light accumulation target
// -----------------------------------------------------------------------------
layout (location = 0) out vec4 out_Output;

// -----------------------------------------------------------------------------
// Tracing and intersection
// -----------------------------------------------------------------------------
struct SRay
{
    vec3 origin;
    vec3 dir;
};

struct SRect
{
    vec3  center;
    vec3  dirx;
    vec3  diry;
    float halfx;
    float halfy;

    vec4  plane;
};

bool RayPlaneIntersect(in SRay _Ray, in vec4 _Plane, out float _Distance)
{
    _Distance = -dot(_Plane, vec4(_Ray.origin, 1.0f)) / dot(_Plane.xyz, _Ray.dir);

    return _Distance > 0.0;
}

bool RayRectIntersect(SRay ray, SRect rect, out float t)
{
    bool intersect = RayPlaneIntersect(ray, rect.plane, t);
    if (intersect)
    {
        vec3 pos  = ray.origin + ray.dir*t;
        vec3 lpos = pos - rect.center;
        
        float x = dot(lpos, rect.dirx);
        float y = dot(lpos, rect.diry);    

        if (abs(x) > rect.halfx || abs(y) > rect.halfy)
        {
            intersect = false;
        }
    }

    return intersect;
}

// -----------------------------------------------------------------------------
// Camera functions
// -----------------------------------------------------------------------------
vec3 mul(mat3 m, vec3 v)
{
    return m * v;
}

mat3 mul(mat3 m1, mat3 m2)
{
    return m1 * m2;
}

vec3 rotation_y(vec3 v, float a)
{
    vec3 r;

    r.x =  v.x*cos(a) + v.z*sin(a);
    r.y =  v.y;
    r.z = -v.x*sin(a) + v.z*cos(a);

    return r;
}

vec3 rotation_z(vec3 v, float a)
{
    vec3 r;

    r.x =  v.x*cos(a) - v.y*sin(a);
    r.y =  v.x*sin(a) + v.y*cos(a);
    r.z =  v.z;

    return r;
}

vec3 rotation_yz(vec3 v, float ay, float az)
{
    return rotation_z(rotation_y(v, ay), az);
}

mat3 transpose(mat3 v)
{
    mat3 tmp;

    tmp[0] = vec3(v[0].x, v[1].x, v[2].x);
    tmp[1] = vec3(v[0].y, v[1].y, v[2].y);
    tmp[2] = vec3(v[0].z, v[1].z, v[2].z);

    return tmp;
}

// -----------------------------------------------------------------------------
// Linearly Transformed Cosines
// -----------------------------------------------------------------------------
float IntegrateEdge(vec3 v1, vec3 v2)
{
    float cosTheta = dot(v1, v2);
    float theta = acos(cosTheta);    
    float res = cross(v1, v2).z * ((theta > 0.001) ? theta/sin(theta) : 1.0);

    return res;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // detect clipping config
    int config = 0;
    if (L[0].z > 0.0) config += 1;
    if (L[1].z > 0.0) config += 2;
    if (L[2].z > 0.0) config += 4;
    if (L[3].z > 0.0) config += 8;

    // clip
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] =  L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }
    
    if (n == 3)
        L[3] = L[0];
    if (n == 4)
        L[4] = L[0];
}


vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = mul(Minv, transpose(mat3(T1, T2, N)));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[5];
    L[0] = mul(Minv, points[0] - P);
    L[1] = mul(Minv, points[1] - P);
    L[2] = mul(Minv, points[2] - P);
    L[3] = mul(Minv, points[3] - P);

    int n;
    ClipQuadToHorizon(L, n);
    
    if (n == 0)
    {
        return vec3(0, 0, 0);
    }

    // project onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    L[4] = normalize(L[4]);

    // integrate
    float sum = 0.0;

    sum += IntegrateEdge(L[0], L[1]);
    sum += IntegrateEdge(L[1], L[2]);
    sum += IntegrateEdge(L[2], L[3]);

    if (n >= 4)
    {
        sum += IntegrateEdge(L[3], L[4]);
    }

    if (n == 5)
    {
        sum += IntegrateEdge(L[4], L[0]);
    }

    sum = twoSided ? abs(sum) : max(0.0, sum);

    vec3 Lo_i = vec3(sum, sum, sum);

    return Lo_i;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void main()
{
    // -----------------------------------------------------------------------------
    // Get data
    // -----------------------------------------------------------------------------
    vec4  GBuffer0 = texture(ps_GBuffer0    , in_TexCoord);
    vec4  GBuffer1 = texture(ps_GBuffer1    , in_TexCoord);
    vec4  GBuffer2 = texture(ps_GBuffer2    , in_TexCoord);
    float VSDepth  = texture(ps_DepthTexture, in_TexCoord).r;

    // -----------------------------------------------------------------------------
    // VS position
    // -----------------------------------------------------------------------------
    vec3 VSPosition = GetViewSpacePositionFromDepth(VSDepth, in_TexCoord, g_ScreenToView);
    
    // -----------------------------------------------------------------------------
    // WS position
    // -----------------------------------------------------------------------------
    vec3 WSPosition = (g_ViewToWorld * vec4(VSPosition, 1.0f)).xyz;

    // -----------------------------------------------------------------------------
    // Surface data
    // -----------------------------------------------------------------------------
    SSurfaceData Data;

    UnpackGBuffer(GBuffer0, GBuffer1, GBuffer2, WSPosition.xyz, VSDepth, Data);

    // -----------------------------------------------------------------------------
    // Exposure data
    // -----------------------------------------------------------------------------
    float AverageExposure = ps_ExposureHistory[ps_ExposureHistoryIndex];

    // -----------------------------------------------------------------------------
    // Create default rect in world
    // -----------------------------------------------------------------------------
    SRect rect;
    vec3  points[4];
    
    vec3 LightDirection = -normalize(vec3(-2.0f, -2.0f, -1.0f));
    vec3 Left           =  normalize(vec3(0.0f, rotz, 1.0f));
    vec3 Right          =  normalize(cross(LightDirection, Left));
    
    Left = cross(LightDirection, Right);
    
    vec3 rectNormal = LightDirection;
    
    rect.dirx = Right;
    rect.diry = Left;
    
    rect.center = vec3(0.0f, 0.0f, 10.0f);
    rect.halfx  = 0.5f * 8;
    rect.halfy  = 0.5f * 8;

    rect.plane = vec4(rectNormal, -dot(rectNormal, rect.center));

    vec3 ex = rect.halfx * rect.dirx;
    vec3 ey = rect.halfy * rect.diry;

    points[0] = rect.center - ex - ey;
    points[1] = rect.center + ex - ey;
    points[2] = rect.center + ex + ey;
    points[3] = rect.center - ex + ey;

    // -----------------------------------------------------------------------------
    // LTC
    // -----------------------------------------------------------------------------
    vec3 Output = vec3(0);

    vec3 WSViewDirection = normalize(g_ViewPosition.xyz - Data.m_WSPosition);
    
    float Theta = acos(dot(Data.m_WSNormal, WSViewDirection));

    vec2 UV = vec2(Data.m_Roughness, Theta / (0.5f * pi));

    UV = UV * LUT_SCALE + LUT_BIAS;
    
    vec4 t = texture2D(ltc_mat, UV);

    mat3 Minv = mat3(
        vec3(  1,   0, t.y),
        vec3(  0, t.z,   0),
        vec3(t.w,   0, t.x)
    );
    
    vec3 Specular = LTC_Evaluate(Data.m_WSNormal, WSViewDirection, WSPosition, Minv, points, false);

    Specular *= texture2D(ltc_mag, UV).w;
    
    vec3 Diffuse = LTC_Evaluate(Data.m_WSNormal, WSViewDirection, WSPosition, mat3(1), points, false); 
    
    Output  = vec3(intensity) * (scolor.xyz * Specular + dcolor.xyz * Diffuse);

    Output /= 2.0f * pi;

    // -----------------------------------------------------------------------------
    // Generate ray from camera
    // -----------------------------------------------------------------------------
#define SHOW_BULB 0
#if SHOW_BULB == 1
    SRay ray;

    ray.origin = g_ViewPosition.xyz;
    ray.dir    = g_ViewDirection.xyz;

    float distToFloor = dot(Data.m_WSPosition, g_ViewPosition.xyz);

    float distToRect;

    if (RayRectIntersect(ray, rect, distToRect))
    {
        if ((distToRect < distToFloor))
        {
            Output = vec3(intensity);
        }
    }
#endif

    out_Output = vec4(Output, 0.0f);
}

#endif // __INCLUDE_FS_LIGHT_AREALIGHT_GLSL__