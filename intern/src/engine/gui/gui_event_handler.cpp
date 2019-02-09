
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

        void OnUserEvent(const Base::CInputEvent& _rEvent);

    public:

		CInputEventDelegate::HandleType RegisterDirectUserListener(CInputEventDelegate::FunctionType _Function);
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

    void CGUIEventHandler::OnUserEvent(const Base::CInputEvent& _rEvent)
    {
		CInputEventDelegate::Notify(_rEvent);
    }

    // -----------------------------------------------------------------------------

	CInputEventDelegate::HandleType CGUIEventHandler::RegisterDirectUserListener(CInputEventDelegate::FunctionType _Function)
    {
		return CInputEventDelegate::Register(_Function);
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

	CInputEventDelegate::HandleType RegisterDirectUserListener(CInputEventDelegate::FunctionType _Function)
    {
        return CGUIEventHandler::GetInstance().RegisterDirectUserListener(_Function);
    }
} // namespace EventHandler
} // namespace Gui
