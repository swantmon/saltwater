#pragma once

#include "base/base_include_glm.h"

namespace AR
{
    class CTarget
    {
    public:

        enum ETrackingState
        {
            Lost,
            Tracking,
            Undefined
        };

    public:

        CTarget();
        ~CTarget();

        ETrackingState GetTackingState() const;
        const glm::mat4& GetModelMatrix() const;

    protected:

        ETrackingState m_TrackingState;
        glm::mat4 m_ModelMatrix;
    };
} // namespace AR