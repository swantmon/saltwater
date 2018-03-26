
#include "mr/mr_precompiled.h"

#include "mr/mr_session.h"

namespace MR
{
    CSession::CSession()
        : m_State   (Undefined)
        , m_pSession(0)
        , m_pFrame  (0)
    {

    }

    // -----------------------------------------------------------------------------

    CSession::~CSession()
    {

    }

    // -----------------------------------------------------------------------------

    CSession::ESessionState CSession::GetSessionState() const
    {
        return m_State;
    }

    // -----------------------------------------------------------------------------

    void* CSession::GetSession() const
    {
        return m_pSession;
    }

    // -----------------------------------------------------------------------------

    void* CSession::GetFrame() const
    {
        return m_pFrame;
    }
} // namespace MR