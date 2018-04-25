
#pragma once

#include "base/base_include_glm.h"

namespace AR
{
    class CCamera
    {
    public:

        enum ETrackingState
        {
            Paused,
            Stopped,
            Tracking,
            Undefined
        };

    public:

        CCamera();
        ~CCamera();

        ETrackingState GetTackingState() const;
        const glm::mat4& GetViewMatrix() const;
        const glm::mat4& GetProjectionMatrix() const;
        float GetNear() const;
        float GetFar() const;

     protected:

        ETrackingState m_TrackingState;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;
        float m_Near;
        float m_Far;
    };
} // namespace AR