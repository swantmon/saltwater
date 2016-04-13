
#include "base/base_console.h"
#include "base/base_input_event.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_fly_control.h"
#include "camera/cam_ar_control.h"

#include "gui/gui_event_handler.h"

#include "graphic/gfx_camera_interface.h"

#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

using namespace Cam;

namespace
{
    class CCamControlManager : private Base::CUncopyable
    {
        BASE_SINGLETON_FUNC(CCamControlManager);
        
    public:

        CCamControlManager();
       ~CCamControlManager();

    public:

        void CreateControl(CControl::EType _Type);

        void SetActiveControl(CControl::EType _Type);
        CControl& GetActiveControl();
        const CControl& GetActiveControl() const;

    public:

        void OnEvent(const Base::CInputEvent& _rEvent);

    public:

        void Update();

    private:

        CControl* m_pActiveControl;
        CControl* m_pControls[CControl::NumberOfControls];
    };
} // namespace

namespace
{
    CCamControlManager::CCamControlManager()
    {
        // -----------------------------------------------------------------------------
        // register input event to gui
        // -----------------------------------------------------------------------------
        Gui::EventHandler::RegisterDirectUserListener(GUI_BIND_INPUT_METHOD(&CCamControlManager::OnEvent));
    }

    // -----------------------------------------------------------------------------

    CCamControlManager::~CCamControlManager()
    {
        for (unsigned int IndexOfControl = 0; IndexOfControl < CControl::NumberOfControls; ++ IndexOfControl)
        {
            Base::CMemory::DeleteObject(m_pControls[IndexOfControl]);
        }
    }

    // -----------------------------------------------------------------------------

    void CCamControlManager::CreateControl(CControl::EType _Type)
    {
        if (m_pControls[_Type] == nullptr)
        {
            switch (_Type)
            {
                case CControl::FlyControl:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CFlyControl>();
                }
                break;
                case CControl::ARControl:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CARControl>();
                }
                break;
                    
                    
                default:
                    BASE_CONSOLE_STREAMWARNING("Can't create camera control of type " << _Type);
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CCamControlManager::SetActiveControl(CControl::EType _Type)
    {
        assert((_Type >= 0) && (_Type < CControl::NumberOfControls));
        
        m_pActiveControl = m_pControls[_Type];
    }

    // -----------------------------------------------------------------------------

    CControl& CCamControlManager::GetActiveControl()
    {
        assert(m_pActiveControl != nullptr);

        return *m_pActiveControl;
    }

    // -----------------------------------------------------------------------------

    const CControl& CCamControlManager::GetActiveControl() const
    {
        assert(m_pActiveControl != nullptr);
        
        return *m_pActiveControl;
    }
    
    // -----------------------------------------------------------------------------

    void CCamControlManager::OnEvent(const Base::CInputEvent& _rEvent)
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->OnEvent(_rEvent);
        }
    }

    // -----------------------------------------------------------------------------

    void CCamControlManager::Update()
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->Update();
        }
    }
} // namespace

namespace Cam
{
namespace ControlManager
{
    void CreateControl(CControl::EType _Type)
    {
        CCamControlManager::GetInstance().CreateControl(_Type);
    }

    // -----------------------------------------------------------------------------

    void SetActiveControl(CControl::EType _Type)
    {
        CCamControlManager::GetInstance().SetActiveControl(_Type);
    }

    // -----------------------------------------------------------------------------

    CControl& GetActiveControl()
    {
        return CCamControlManager::GetInstance().GetActiveControl();
    }

    // -----------------------------------------------------------------------------

    void Update()
    {
        CCamControlManager::GetInstance().Update();
    }
} // namespace ControlManager
} // namespace Cam
