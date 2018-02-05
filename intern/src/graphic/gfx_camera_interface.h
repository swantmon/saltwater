
#pragma once

#include "base/base_aabb2.h"

#include "glm.hpp"

namespace Gfx
{
namespace Cam
{    
    // -----------------------------------------------------------------------------
    // Camera
    // -----------------------------------------------------------------------------
    void SetFieldOfView(float _FOVY, float _Near, float _Far);
    void SetPerspective(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    void SetOrthographic(float _Left, float _Right, float _Bottom, float _Top, float _Near, float _Far);
    void SetProjectionMatrix(const glm::mat4& _rProjectionMatrix, float _Near, float _Far);

    void SetBackgroundColor(glm::vec3& _rBackgroundColor);
    const glm::vec3& GetBackgroundColor();

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
    void SetRotationMatrix(const glm::mat3& _rMatrix);
    const glm::mat3& GetRotationMatrix();

    void SetPosition(float _AxisX, float _AxisY, float _AxisZ);
    void SetPosition(const glm::vec3& _rPosition);
    const glm::vec3& GetPosition();

    void SetViewMatrix(const glm::mat4& _rViewMatrix);
    const glm::mat4& GetViewMatrix();

    const glm::mat4& GetProjectionMatrix();

    // -----------------------------------------------------------------------------
    // Manager
    // -----------------------------------------------------------------------------
    void Update();
} // namespace Cam
} // namespace Gfx