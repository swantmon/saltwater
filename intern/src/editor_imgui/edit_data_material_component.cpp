
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_base_factory.h"
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_material_component.h"

namespace Dt
{
    class CMaterialComponentGUIFactory : public Edit::CBaseFactory
    {
    public:
        CMaterialComponentGUIFactory() { }
        CMaterialComponentGUIFactory(void* _pEntity) { m_GUI = (CMaterialComponentGUI*)(_pEntity); };
        void* Create(void* _pChild) { return new CMaterialComponentGUIFactory(_pChild); };
        void OnGUI() { m_GUI->OnGUI(); }
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
