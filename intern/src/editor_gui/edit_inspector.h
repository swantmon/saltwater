#pragma once

#include "editor_gui/edit_inspector_bloom.h"
#include "editor_gui/edit_inspector_dof.h"
#include "editor_gui/edit_inspector_entity.h"
#include "editor_gui/edit_inspector_environment.h"
#include "editor_gui/edit_inspector_fxaa.h"
#include "editor_gui/edit_inspector_globalprobe.h"
#include "editor_gui/edit_inspector_pointlight.h"
#include "editor_gui/edit_inspector_sun.h"
#include "editor_gui/edit_inspector_ssr.h"
#include "editor_gui/edit_inspector_transformation.h"
#include "editor_gui/edit_inspector_volumefog.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspector : public QWidget
    {
        Q_OBJECT

    public:

        CInspector(QWidget* _pParent = Q_NULLPTR);
        ~CInspector();

    public Q_SLOTS:

        void updateContentForEntity(int _ID);

    private:

        QVBoxLayout*              m_pInspectorLayout;
        CInspectorEntity*         m_pEntityWidget;
        CInspectorPointlight*     m_pPointlightWidget;
        CInspectorSun*            m_pSunWidget;
        CInspectorTransformation* m_pTransformWidget;
        CInspectorEnvironment*    m_pEnvironmentWidget;
        CInspectorGlobalProbe*    m_pGlobalProbeWidget;
        CInspectorBloom*          m_pBloomWidget;
        CInspectorDOF*            m_pDOFWidget;
        CInspectorFXAA*           m_pFXAAWidget;
        CInspectorSSR*            m_pSSRWidget;
        CInspectorVolumeFog*      m_pVolumeFogWidget;

    private:

        void OnEntityInfoFacets(Edit::CMessage& _rMessage);
    };
} // namespace Edit