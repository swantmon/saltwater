
#pragma once

#include "base/base_aabb2.h"
#include "base/base_matrix3x3.h"
#include "base/base_matrix4x4.h"
#include "base/base_vector2.h"
#include "base/base_vector3.h"

namespace Gfx
{
namespace Cam
{    
    // -----------------------------------------------------------------------------
    // Camera
    // -----------------------------------------------------------------------------
    void SetFieldOfView(float _FOVY, float _Near, float _Far);
    void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);

    void SetProjection(const Base::Float4x4& _rProjectionMatrix);

    void SetBackgroundColor(Base::Float3& _rBackgroundColor);
    const Base::Float3& GetBackgroundColor();

    void SetCullingMask(unsigned int _CullingMask);
    unsigned int GetCullingMask();

    void SetViewportRect(Base::AABB2Float& _rViewportRect);
    const Base::AABB2Float& GetViewportRect();

    void SetDepth(float _Depth);
    float GetDepth();

    void SetAutoCameraMode();
    void SetManualCameraMode();

    void SetShutterSpeed(float _ShutterSpeed);
    float GetShutterSpeed();

    void SetAperture(float _Aperture);
    float GetAperture();

    void SetISO(float _ISO);
    float GetISO();

    void SetEC(float _EC);
    float GetEC();

    // -----------------------------------------------------------------------------
    // View
    // -----------------------------------------------------------------------------
    void SetRotationMatrix();
    void SetRotationMatrix(const Base::Float3x3& _rMatrix);
    const Base::Float3x3& GetRotationMatrix();

    void SetPosition(float _X, float _Y, float _Z);
    void SetPosition(const Base::Float3& _rPosition);
    const Base::Float3& GetPosition();

    void SetViewMatrix(const Base::Float4x4& _rViewMatrix);
    const Base::Float4x4& GetViewMatrix();

    // -----------------------------------------------------------------------------
    // Manager
    // -----------------------------------------------------------------------------
    void Update();
} // namespace Cam
} // namespace Gfx