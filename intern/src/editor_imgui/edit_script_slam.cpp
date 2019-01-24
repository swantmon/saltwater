
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_script_slam.h"

namespace Edit
{
    class CSLAMScriptGUIFactory : public Edit::IGUIFactory
    {
    public:
        CSLAMScriptGUIFactory() { }
        IGUIFactory* Create() { return new CSLAMScriptGUIFactory(); };
        void SetChild(void* _pChild) { m_GUI = (CSLAMScriptGUI*)(_pChild); }
        void OnGUI() { m_GUI->OnGUI(); }
        const char* GetHeader() { return m_GUI->GetHeader(); };
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
