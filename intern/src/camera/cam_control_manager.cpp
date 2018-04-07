
#include "engine/engine_precompiled.h"

#include "base/base_console.h"
#include "base/base_input_event.h"
#include "base/base_memory.h"
#include "base/base_uncopyable.h"
#include "base/base_singleton.h"

#include "camera/cam_control_manager.h"
#include "camera/cam_editor_control.h"
#include "camera/cam_game_control.h"

#include "data/data_entity_manager.h"
#include "data/data_component_manager.h"

#include "gui/gui_event_handler.h"

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
        void OnDirtyEntity(Dt::CEntity* _pEntity);
        void OnDirtyComponent(Dt::IComponent* _pComponent);

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
        // register changing components
        // -----------------------------------------------------------------------------
        Dt::CComponentManager::GetInstance().RegisterDirtyComponentHandler(DATA_DIRTY_COMPONENT_METHOD(&CCamControlManager::OnDirtyComponent));

        // -----------------------------------------------------------------------------
        // register changing entities
        // -----------------------------------------------------------------------------
        Dt::EntityManager::RegisterDirtyEntityHandler(DATA_DIRTY_ENTITY_METHOD(&CCamControlManager::OnDirtyEntity));

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
            if (m_pControls[IndexOfControl] != nullptr)
            {
                Base::CMemory::DeleteObject(m_pControls[IndexOfControl]);
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CCamControlManager::CreateControl(CControl::EType _Type)
    {
        if (m_pControls[_Type] == nullptr)
        {
            switch (_Type)
            {
                case CControl::EditorControl:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CEditorControl>();
                }
                break;
                case CControl::GameControl:
                {
                    m_pControls[_Type] = Base::CMemory::NewObject<CGameControl>();
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

    void CCamControlManager::OnDirtyEntity(Dt::CEntity* _pEntity)
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->OnDirtyEntity(_pEntity);
        }
    }

    // -----------------------------------------------------------------------------

    void CCamControlManager::OnDirtyComponent(Dt::IComponent* _pComponent)
    {
        if (m_pActiveControl != nullptr)
        {
            m_pActiveControl->OnDirtyComponent(_pComponent);
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
