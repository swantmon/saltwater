
#include "mr/mr_precompiled.h"

#include "mr/mr_session.h"

namespace MR
{
    CSession::CSession()
        : m_State             (Undefined)
        , m_pSession          (0)
        , m_pFrame            (0)
        , m_HasGeometryChanged(false)
        , m_UVs               ({ glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f) })
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

    // -----------------------------------------------------------------------------

    bool CSession::HasGeometryChanged() const
    {
        return m_HasGeometryChanged;
    }

    // -----------------------------------------------------------------------------

    CSession::CUVs CSession::GetTransformedUVs() const
    {
        return m_UVs;
    }
} // namespace MR