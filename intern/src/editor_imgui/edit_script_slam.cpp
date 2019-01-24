
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_script_slam.h"

namespace Edit
{
    class CSLAMScriptGUIFactory : public Edit::IGUIFactory
    {
    public:
        CSLAMScriptGUIFactory() { }
        CSLAMScriptGUIFactory(void* _pEntity) { m_GUI = (CSLAMScriptGUI*)(_pEntity); };
        void* Create(void* _pChild) { return new CSLAMScriptGUIFactory(_pChild); };
        void OnGUI() { m_GUI->OnGUI(); }
    private:
        CSLAMScriptGUI* m_GUI;
    };
    struct SRegFactoryCSLAMScriptGUIFactory
    {
        SRegFactoryCSLAMScriptGUIFactory()
        {
            static CSLAMScriptGUIFactory s_CSLAMScriptGUIFactory;
            Edit::CGUIFactory::GetInstance().Register<Scpt::CSLAMScript>(&s_CSLAMScriptGUIFactory);
        }
    } const g_SRegFactoryCSLAMScriptGUIFactory;
} // namespace Edit
