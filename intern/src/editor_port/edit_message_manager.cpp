
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <assert.h>
#include <vector>

using namespace Edit;

namespace
{
    class CEditMessageManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CEditMessageManager)

    public:

        void Register(SGUIMessageType::Enum _Type, const CMessageDelegate& _rDelegate);
        void Register(SApplicationMessageType::Enum _Type, const CMessageDelegate& _rDelegate);

    public:

        int SendMessage(SGUIMessageType::Enum _Type, const CMessage& _rMessage);
        int SendMessage(SApplicationMessageType::Enum _Type, const CMessage& _rMessage);

    private:

        typedef std::vector<CMessageDelegate>    CMessageDelegateVector;
        typedef CMessageDelegateVector::iterator CMessageDelegateIterator;

    private:

        CMessageDelegateVector m_GUIMessageDelegates[SGUIMessageType::NumberOfMembers];
        CMessageDelegateVector m_ApplicationMessageDelegates[SApplicationMessageType::NumberOfMembers];

    private:

        CEditMessageManager();
        ~CEditMessageManager();
    };
} // namespace

namespace
{
    CEditMessageManager::CEditMessageManager()
    {
        unsigned int IndexOfMessageType;

        for (IndexOfMessageType = 0; IndexOfMessageType < SGUIMessageType::NumberOfMembers; ++ IndexOfMessageType)
        {
            m_GUIMessageDelegates[IndexOfMessageType].reserve(16);
        }

        for (IndexOfMessageType = 0; IndexOfMessageType < SApplicationMessageType::NumberOfMembers; ++IndexOfMessageType)
        {
            m_ApplicationMessageDelegates[IndexOfMessageType].reserve(16);
        }
    }

    // -----------------------------------------------------------------------------

    CEditMessageManager::~CEditMessageManager()
    {
    }

    // -----------------------------------------------------------------------------

    void CEditMessageManager::Register(SGUIMessageType::Enum _Type, const CMessageDelegate& _rDelegate)
    {
        m_GUIMessageDelegates[_Type].push_back(_rDelegate);
    }

    // -----------------------------------------------------------------------------

    void CEditMessageManager::Register(SApplicationMessageType::Enum _Type, const CMessageDelegate& _rDelegate)
    {
        m_ApplicationMessageDelegates[_Type].push_back(_rDelegate);
    }

    // -----------------------------------------------------------------------------

    int CEditMessageManager::SendMessage(SGUIMessageType::Enum _Type, const CMessage& _rMessage)
    {
        assert((_Type >= 0) && (_Type < SGUIMessageType::NumberOfMembers));

        CMessage Message = _rMessage;

        Message.Proof();

        CMessageDelegateVector& rDelegates = m_GUIMessageDelegates[_Type];

        const CMessageDelegateIterator DelegatesEndIterator = rDelegates.end();

        for (CMessageDelegateIterator DelegatesIterator = rDelegates.begin(); DelegatesIterator != DelegatesEndIterator; ++ DelegatesIterator)
        {
            (*DelegatesIterator)(Message);

            Message.Reset();
        }

        return Message.GetResult();
    }

    // -----------------------------------------------------------------------------

    int CEditMessageManager::SendMessage(SApplicationMessageType::Enum _Type, const CMessage& _rMessage)
    {
        assert((_Type >= 0) && (_Type < SGUIMessageType::NumberOfMembers));

        CMessage Message = _rMessage;

        Message.Proof();

        CMessageDelegateVector& rDelegates = m_ApplicationMessageDelegates[_Type];

        const CMessageDelegateIterator DelegatesEndIterator = rDelegates.end();

        for (CMessageDelegateIterator DelegatesIterator = rDelegates.begin(); DelegatesIterator != DelegatesEndIterator; ++DelegatesIterator)
        {
            (*DelegatesIterator)(Message);

            Message.Reset();
        }

        return Message.GetResult();
    }
} // namespace

namespace Edit
{
namespace MessageManager
{
    void Register(SGUIMessageType::Enum _Type, const CMessageDelegate& _rDelegate)
    {
        CEditMessageManager::GetInstance().Register(_Type, _rDelegate);
    }

    // -----------------------------------------------------------------------------

    void Register(SApplicationMessageType::Enum _Type, const CMessageDelegate& _rDelegate)
    {
        CEditMessageManager::GetInstance().Register(_Type, _rDelegate);
    }

    // -----------------------------------------------------------------------------

    int SendMessage(SGUIMessageType::Enum _Type, const CMessage& _rMessage)
    {
        return CEditMessageManager::GetInstance().SendMessage(_Type, _rMessage);
    }

    // -----------------------------------------------------------------------------

    int SendMessage(SApplicationMessageType::Enum _Type, const CMessage& _rMessage)
    {
        return CEditMessageManager::GetInstance().SendMessage(_Type, _rMessage);
    }
} // namespace MessageManager
} // namespace Edit
