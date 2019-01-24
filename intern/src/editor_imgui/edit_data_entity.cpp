
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_entity.h"

namespace Dt
{
    class CEntityGUIFactory : public Edit::IGUIFactory
    {
    public:
        CEntityGUIFactory() { }
        CEntityGUIFactory(void* _pEntity) { m_GUI = (CEntityGUI*)(_pEntity);};
        void* Create(void* _pChild) { return new CEntityGUIFactory(_pChild); };
        void OnGUI() { m_GUI->OnGUI(); }
    private:
        CEntityGUI* m_GUI;
    };
    struct SRegFactoryCEntityGUIFactory
    {
        SRegFactoryCEntityGUIFactory()
        {
            static CEntityGUIFactory s_CEntityGUIFactory;
            Edit::CGUIFactory::GetInstance().Register<CEntity>(&s_CEntityGUIFactory);
        }
    } const g_SRegFactoryCEntityGUIFactory;        
} // namespace Dt
