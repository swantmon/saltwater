
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_entity.h"

namespace Dt
{
    class CEntityGUIFactory : public Edit::IGUIFactory
    {
    public:
        CEntityGUIFactory() { }
        IGUIFactory* Create() { return new CEntityGUIFactory(); };
        void SetChild(void* _pChild) { m_GUI = (CEntityGUI*)(_pChild); }
        void OnGUI() { m_GUI->OnGUI(); }
        const char* GetHeader() { return m_GUI->GetHeader(); };
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
