
#include "editor_imgui/edit_precompiled.h"

#include "editor_imgui/edit_base_factory.h"
#include "editor_imgui/edit_gui_factory.h"
#include "editor_imgui/edit_data_transformation_facet.h"

namespace Dt
{
    class CTransformationFacetGUIFactory : public Edit::CBaseFactory
    {
    public:
        CTransformationFacetGUIFactory() { }
        CTransformationFacetGUIFactory(void* _pEntity) { m_GUI = (CTransformationFacetGUI*)(_pEntity); };
        void* Create(void* _pChild) { return new CTransformationFacetGUIFactory(_pChild); };
        void OnGUI() { m_GUI->OnGUI(); }
    private:
        CTransformationFacetGUI* m_GUI;
    };
    struct SRegFactoryCTransformationFacetGUIFactory
    {
        SRegFactoryCTransformationFacetGUIFactory()
        {
            static CTransformationFacetGUIFactory s_CTransformationFacetGUIFactory;
            Edit::CGUIFactory::GetInstance().RegisterClass<CTransformationFacet>(&s_CTransformationFacetGUIFactory);
        }
    } const g_SRegFactoryCTransformationFacetGUIFactory;
} // namespace Dt
