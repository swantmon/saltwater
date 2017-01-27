#ifndef __INCLUDE_RAYCAST_GLSL__
#define __INCLUDE_RAYCAST_GLSL__

struct SRay
{
    vec3 m_Origin;
    vec3 m_Direction;
};

// -----------------------------------------------------------------------------

struct SRectangle
{
    vec3  m_Center;
    vec3  m_DirectionX;
    vec3  m_DirectionY;
    float m_HalfWidth;
    float m_HalfHeight;

    vec4  m_Normal;
};

// -----------------------------------------------------------------------------

bool RayPlaneIntersect(in SRay _Ray, in vec4 _PlaneNormal, out float _Distance)
{
    _Distance = -dot(_PlaneNormal, vec4(_Ray.m_Origin, 1.0f)) / dot(_PlaneNormal.xyz, _Ray.m_Direction);

    return _Distance > 0.0;
}

// -----------------------------------------------------------------------------

bool RayRectIntersect(in SRay _Ray, in SRectangle _Rectangle, out float _Distance)
{
    bool IsIntersecting = RayPlaneIntersect(_Ray, _Rectangle.m_Normal, _Distance);

    if (IsIntersecting)
    {
        vec3 WSPosition      = _Ray.m_Origin + _Ray.m_Direction * _Distance;
        vec3 WSViewDirection = WSPosition - _Rectangle.m_Center;
        
        float X = dot(WSViewDirection, _Rectangle.m_DirectionX);
        float Y = dot(WSViewDirection, _Rectangle.m_DirectionY);    

        if (abs(X) > _Rectangle.m_HalfWidth || abs(Y) > _Rectangle.m_HalfHeight)
        {
            IsIntersecting = false;
        }
    }

    return IsIntersecting;
}

#endif // __INCLUDE_RAYCAST_GLSL__