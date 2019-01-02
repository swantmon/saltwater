
#include "engine/engine_precompiled.h"

#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/gui/gui_event_handler.h"

#include <assert.h>
#include <vector>

using namespace Gui;

namespace
{
    class CGUIEventHandler : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGUIEventHandler)
        
    public:

        CGUIEventHandler();
        ~CGUIEventHandler();
    
    public:

        void OnUserEvent(const Base::CInputEvent& _rEvent);
        void OnUpdateEvent();

    public:

        void RegisterDirectUserListener(const CInputEventDelegate& _rListener);
        void UnregisterDirectUserListener(const CInputEventDelegate& _rListener);
        void UnregisterAllDirectUserListeners();
        bool ContainsDirectUserListener(const CInputEventDelegate& _rListener) const;

    private:

        typedef std::vector<CInputEventDelegate> CDelegates;
        typedef CDelegates::iterator             CDelegateIterator;
        typedef CDelegates::const_iterator       CDelegateConstIterator;

    private:

        CDelegates  m_DirectUserListeners;
        
    private:

        CDelegateConstIterator GetDirectUserListener(const CInputEventDelegate& _rListener) const;
    };
} // namespace

namespace
{
    CGUIEventHandler::CGUIEventHandler()
        : m_DirectUserListeners()
    {
        m_DirectUserListeners.reserve(256);
    }

	// -----------------------------------------------------------------------------

    CGUIEventHandler::~CGUIEventHandler()
    {
    }

    // -----------------------------------------------------------------------------

    void CGUIEventHandler::OnUserEvent(const Base::CInputEvent& _rEvent)
    {
        // -----------------------------------------------------------------------------
        // Inform all direct user listeners about the event.
        // -----------------------------------------------------------------------------
        const CDelegateConstIterator EndOfDirectUserListeners = m_DirectUserListeners.end();

        for (CDelegateConstIterator CurrentDirectUserListener = m_DirectUserListeners.begin(); CurrentDirectUserListener < EndOfDirectUserListeners; ++ CurrentDirectUserListener)
        {
            (*CurrentDirectUserListener)(_rEvent);
        }
    }

    // -----------------------------------------------------------------------------

    void CGUIEventHandler::RegisterDirectUserListener(const CInputEventDelegate& _rListener)
    {
        assert(!ContainsDirectUserListener(_rListener));

        m_DirectUserListeners.push_back(_rListener);
    }

    // -----------------------------------------------------------------------------

    void CGUIEventHandler::UnregisterDirectUserListener(const CInputEventDelegate& _rListener)
    {
        CDelegateConstIterator DirectUserListener = GetDirectUserListener(_rListener);

        assert(DirectUserListener != m_DirectUserListeners.end());

        m_DirectUserListeners.erase(DirectUserListener);
    }

    // -----------------------------------------------------------------------------

    void CGUIEventHandler::UnregisterAllDirectUserListeners()
    {
        m_DirectUserListeners.clear();
    }

    // -----------------------------------------------------------------------------

    CGUIEventHandler::CDelegateConstIterator CGUIEventHandler::GetDirectUserListener(const CInputEventDelegate& _rListener) const
    {
        CDelegateConstIterator EndOfDirectUserListeners = m_DirectUserListeners.end();

        for (CDelegateConstIterator CurrentDirectUserListener = m_DirectUserListeners.begin(); CurrentDirectUserListener < EndOfDirectUserListeners; ++ CurrentDirectUserListener)
        {
            if ((*CurrentDirectUserListener).target_type() == _rListener.target_type())
            {
                return CurrentDirectUserListener;
            }
        }

        return EndOfDirectUserListeners;
    }

    // -----------------------------------------------------------------------------

    bool CGUIEventHandler::ContainsDirectUserListener(const CInputEventDelegate& _rListener) const
    {
        return GetDirectUserListener(_rListener) != m_DirectUserListeners.end();
    }
} // namespace

namespace Gui
{
namespace EventHandler
{
    void OnUserEvent(const Base::CInputEvent& _rEvent)
    {
        CGUIEventHandler::GetInstance().OnUserEvent(_rEvent);
    }

    // -----------------------------------------------------------------------------

    void RegisterDirectUserListener(const CInputEventDelegate& _rListener)
    {
        CGUIEventHandler::GetInstance().RegisterDirectUserListener(_rListener);
    }

    // -----------------------------------------------------------------------------

    void UnregisterDirectUserListener(const CInputEventDelegate& _rListener)
    {
        CGUIEventHandler::GetInstance().UnregisterDirectUserListener(_rListener);
    }

    // -----------------------------------------------------------------------------

    void UnregisterAllDirectUserListeners()
    {
        CGUIEventHandler::GetInstance().UnregisterAllDirectUserListeners();
    }

    // -----------------------------------------------------------------------------

    bool ContainsDirectUserListener(const CInputEventDelegate& _rListener)
    {
        return CGUIEventHandler::GetInstance().ContainsDirectUserListener(_rListener);
    }
} // namespace EventHandler
} // namespace Gui
