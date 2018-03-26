#pragma once

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

        CSession();
        ~CSession();

        ESessionState GetSessionState() const;
        void* GetSession() const;
        void* GetFrame() const;

    protected:

        ESessionState m_State;
        void* m_pSession;
        void* m_pFrame;
    };
} // namespace MR