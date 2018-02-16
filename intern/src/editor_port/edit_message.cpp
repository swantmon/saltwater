
#include "editor_port/edit_precompiled.h"

#include "editor_port/edit_message.h"

#include <assert.h>

namespace Edit
{
    CMessage::CMessage()
        : m_Result(0)
        , m_Mode  (Write)
        , m_Bytes ()
        , m_Pos   (m_Bytes.begin())
    {
    }
    
    // -----------------------------------------------------------------------------

    CMessage::CMessage(bool _Empty)
        : m_Result(0)
        , m_Mode (Write)
        , m_Bytes()
        , m_Pos  (m_Bytes.begin())
    {
        if (_Empty)
        {
            const Base::Char Terminator = '\0';

            m_Bytes.insert(m_Bytes.end(), reinterpret_cast<const Base::U8*>(&Terminator), reinterpret_cast<const Base::U8*>(&Terminator) + sizeof(Terminator));
        }
    }

    // -----------------------------------------------------------------------------

    void CMessage::Reset()
    {
        m_Pos = m_Bytes.begin();
    }

    // -----------------------------------------------------------------------------

    void CMessage::Proof()
    {
        m_Mode = Read;
    }

    // -----------------------------------------------------------------------------

    void CMessage::SetResult(int _Result)
    {
        m_Result = _Result;
    }

    // -----------------------------------------------------------------------------

    int CMessage::GetResult() const
    {
        return m_Result;
    }

    // -----------------------------------------------------------------------------

    Base::Size CMessage::GetPosition() const
    {
        return m_Pos - const_cast<CByteVector&>(m_Bytes).begin();
    }
    
    // -----------------------------------------------------------------------------

    void CMessage::PutBytes(void* _pBytes, Base::Size _NumberOfBytes)
    {
        assert(m_Mode == Write);

        m_Bytes.insert(m_Bytes.end(), static_cast<Base::U8*>(_pBytes), static_cast<Base::U8*>(_pBytes) + _NumberOfBytes);
    }

    // -----------------------------------------------------------------------------

    Base::Size CMessage::GetNumberOfBytes() const
    {
        return m_Bytes.size();
    }

    // -----------------------------------------------------------------------------

    const void* CMessage::GetBytes() const
    {
        return &m_Bytes.front();
    }
} // namespace Edit
