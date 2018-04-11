#pragma once

#include "base/base_include_glm.h"

namespace MR
{
    class CMarker
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

        CMarker();
        ~CMarker();

        ETrackingState GetTackingState() const;
        const glm::mat4& GetModelMatrix() const;

    protected:

        ETrackingState m_TrackingState;
        glm::mat4 m_ModelMatrix;
    };
} // namespace MR