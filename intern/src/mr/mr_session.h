
#pragma once

#include "base/base_include_glm.h"

#include <array>

namespace MR
{
    class CSession
    {
    public:

        enum ESessionState
        {
            Error,
            Paused,
            TextureNotSet,
            MissingGLContext,
            CameraNotAvailable,
            Success,
            Undefined
        };

    public:

        typedef std::array<glm::vec2, 4> CUVs;

    public:

        CSession();
        ~CSession();

        ESessionState GetSessionState() const;
        void* GetSession() const;
        void* GetFrame() const;
        bool HasGeometryChanged() const;
        CUVs GetTransformedUVs() const;

    protected:

        ESessionState m_State;
        void* m_pSession;
        void* m_pFrame;
        bool m_HasGeometryChanged;
        std::array<glm::vec2, 4> m_UVs;
    };
} // namespace MR