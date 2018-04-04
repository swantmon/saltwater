
#pragma once

#include "base/base_aabb2.h"
#include "base/base_include_glm.h"

#include "graphic/gfx_export.h"

namespace Gfx
{
namespace Cam
{    
    // -----------------------------------------------------------------------------
    // Camera
    // -----------------------------------------------------------------------------
    GFX_API void SetFieldOfView(float _FOVY, float _Near, float _Far);
    GFX_API void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    GFX_API void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    GFX_API void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix, float _Near, float _Far);

    GFX_API void SetBackgroundColor(glm::vec3& _rBackgroundColor);
    GFX_API const glm::vec3& GetBackgroundColor();

    GFX_API void SetCullingMask(unsigned int _CullingMask);
    GFX_API unsigned int GetCullingMask();

    GFX_API void SetViewportRect(Base::AABB2Float& _rViewportRect);
    GFX_API const Base::AABB2Float& GetViewportRect();

    GFX_API void SetDepth(float _Depth);
    GFX_API float GetDepth();

    GFX_API void SetAutoCameraMode();
    GFX_API void SetManualCameraMode();

    GFX_API void SetShutterSpeed(float _ShutterSpeed);
    GFX_API float GetShutterSpeed();

    GFX_API void SetAperture(float _Aperture);
    GFX_API float GetAperture();

    GFX_API void SetISO(float _ISO);
    GFX_API float GetISO();

    GFX_API void SetEC(float _EC);
    GFX_API float GetEC();

    // -----------------------------------------------------------------------------
    // View
    // -----------------------------------------------------------------------------
    GFX_API void SetRotationMatrix(const glm::mat3& _rMatrix);
    GFX_API const glm::mat3& GetRotationMatrix();

    GFX_API void SetPosition(float _AxisX, float _AxisY, float _AxisZ);
    GFX_API void SetPosition(const glm::vec3& _rPosition);
    GFX_API const glm::vec3& GetPosition();

    GFX_API  const glm::mat4& GetViewMatrix();

    GFX_API const glm::mat4& GetProjectionMatrix();

    // -----------------------------------------------------------------------------
    // Manager
    // -----------------------------------------------------------------------------
    GFX_API void Update();
} // namespace Cam
} // namespace Gfx