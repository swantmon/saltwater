
#pragma once

#include "engine/engine_config.h"

#include "base/base_aabb2.h"
#include "base/base_include_glm.h"

namespace Gfx
{
namespace Cam
{    
    // -----------------------------------------------------------------------------
    // Camera
    // -----------------------------------------------------------------------------
    ENGINE_API void SetFieldOfView(float _FOVY, float _Near, float _Far);
    ENGINE_API void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    ENGINE_API void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    ENGINE_API void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix, float _Near, float _Far);

    ENGINE_API void SetBackgroundColor(glm::vec3& _rBackgroundColor);
    ENGINE_API const glm::vec3& GetBackgroundColor();

    ENGINE_API void SetCullingMask(unsigned int _CullingMask);
    ENGINE_API unsigned int GetCullingMask();

    ENGINE_API void SetViewportRect(Base::AABB2Float& _rViewportRect);
    ENGINE_API const Base::AABB2Float& GetViewportRect();

    ENGINE_API void SetDepth(float _Depth);
    ENGINE_API float GetDepth();

    ENGINE_API void SetAutoCameraMode();
    ENGINE_API void SetManualCameraMode();

    ENGINE_API void SetShutterSpeed(float _ShutterSpeed);
    ENGINE_API float GetShutterSpeed();

    ENGINE_API void SetAperture(float _Aperture);
    ENGINE_API float GetAperture();

    ENGINE_API void SetISO(float _ISO);
    ENGINE_API float GetISO();

    ENGINE_API void SetEC(float _EC);
    ENGINE_API float GetEC();

    // -----------------------------------------------------------------------------
    // View
    // -----------------------------------------------------------------------------
    ENGINE_API void SetRotationMatrix(const glm::mat3& _rMatrix);
    ENGINE_API const glm::mat3& GetRotationMatrix();

    ENGINE_API void SetPosition(float _AxisX, float _AxisY, float _AxisZ);
    ENGINE_API void SetPosition(const glm::vec3& _rPosition);
    ENGINE_API const glm::vec3& GetPosition();

    ENGINE_API  const glm::mat4& GetViewMatrix();

    ENGINE_API const glm::mat4& GetProjectionMatrix();

    // -----------------------------------------------------------------------------
    // Manager
    // -----------------------------------------------------------------------------
    ENGINE_API void Update();
} // namespace Cam
} // namespace Gfx