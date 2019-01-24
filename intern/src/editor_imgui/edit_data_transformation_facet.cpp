
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_transformation_facet.h"

namespace Dt
{
    class CTransformationFacetGUIFactory : public Edit::IGUIFactory
    {
    public:
        CTransformationFacetGUIFactory() { }
        IGUIFactory* Create() { return new CTransformationFacetGUIFactory(); };
        void SetChild(void* _pChild) { m_GUI = (CTransformationFacetGUI*)(_pChild); }
        void OnGUI() { m_GUI->OnGUI(); }
        const char* GetHeader() { return m_GUI->GetHeader(); };
    private:
        CTransformationFacetGUI* m_GUI;
    };
    struct SRegFactoryCTransformationFacetGUIFactory
    {
        SRegFactoryCTransformationFacetGUIFactory()
        {
            static CTransformationFacetGUIFactory s_CTransformationFacetGUIFactory;
            Edit::CGUIFactory::GetInstance().Register<CTransformationFacet>(&s_CTransformationFacetGUIFactory);
        }
    } const g_SRegFactoryCTransformationFacetGUIFactory;
} // namespace Dt
