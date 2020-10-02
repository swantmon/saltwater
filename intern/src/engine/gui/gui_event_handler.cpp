
#include "engine/engine_precompiled.h"

#include "base/base_input_event.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "engine/gui/gui_event_handler.h"

#include <assert.h>
#include <vector>

using namespace Gui;
using namespace Gui::EventHandler;

namespace
{
    class CGUIEventHandler : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CGUIEventHandler)
        
    public:

        CGUIEventHandler();
        ~CGUIEventHandler();
    
    public:

        void OnEvent(const Base::CInputEvent& _rEvent);

    public:

        CEventDelegate::HandleType RegisterEventHandler(CEventDelegate::FunctionType _Function);

    private:

        CEventDelegate m_EventDelegate;
    };
} // namespace

namespace
{
    CGUIEventHandler::CGUIEventHandler()
    {
    }

    // -----------------------------------------------------------------------------

    CGUIEventHandler::~CGUIEventHandler()
    {
    }

    // -----------------------------------------------------------------------------

    void CGUIEventHandler::OnEvent(const Base::CInputEvent& _rEvent)
    {
        m_EventDelegate.Notify(_rEvent);
    }

    // -----------------------------------------------------------------------------

    CEventDelegate::HandleType CGUIEventHandler::RegisterEventHandler(CEventDelegate::FunctionType _Function)
    {
        return m_EventDelegate.Register(_Function);
    }
} // namespace

namespace Gui
{
namespace EventHandler
{
    void OnEvent(const Base::CInputEvent& _rEvent)
    {
        CGUIEventHandler::GetInstance().OnEvent(_rEvent);
    }

    // -----------------------------------------------------------------------------

    CEventDelegate::HandleType RegisterEventHandler(CEventDelegate::FunctionType _Function)
    {
        return CGUIEventHandler::GetInstance().RegisterEventHandler(_Function);
    }
} // namespace EventHandler
} // namespace Gui
