
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_material_component.h"

namespace Dt
{
    class CMaterialComponentGUIFactory : public Edit::IGUIFactory
    {
    public:
        CMaterialComponentGUIFactory() { }
        IGUIFactory* Create() { return new CMaterialComponentGUIFactory(); };
        void SetChild(void* _pChild) { m_GUI = (CMaterialComponentGUI*)(_pChild); }
        void OnGUI() { m_GUI->OnGUI(); }
        const char* GetHeader() { return m_GUI->GetHeader(); };
    private:
        CMaterialComponentGUI* m_GUI;
    };
    struct SRegFactoryCMaterialComponentGUIFactory
    {
        SRegFactoryCMaterialComponentGUIFactory()
        {
            static CMaterialComponentGUIFactory s_CMaterialComponentGUIFactory;
            Edit::CGUIFactory::GetInstance().Register<CMaterialComponent>(&s_CMaterialComponentGUIFactory);
        }
    } const g_SRegFactoryCMaterialComponentGUIFactory;
} // namespace Dt
