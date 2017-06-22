#pragma once

#include "editor_gui/ui_edit_inspector.h"

#include "editor_gui/edit_inspector_arcontroller.h"
#include "editor_gui/edit_inspector_arealight.h"
#include "editor_gui/edit_inspector_bloom.h"
#include "editor_gui/edit_inspector_camera.h"
#include "editor_gui/edit_inspector_dof.h"
#include "editor_gui/edit_inspector_entity.h"
#include "editor_gui/edit_inspector_environment.h"
#include "editor_gui/edit_inspector_light_probe.h"
#include "editor_gui/edit_inspector_post_aa.h"
#include "editor_gui/edit_inspector_material.h"
#include "editor_gui/edit_inspector_pointlight.h"
#include "editor_gui/edit_inspector_sun.h"
#include "editor_gui/edit_inspector_ssr.h"
#include "editor_gui/edit_inspector_texture.h"
#include "editor_gui/edit_inspector_transformation.h"
#include "editor_gui/edit_inspector_volumefog.h"
#include "editor_gui/edit_inspector_slam.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspector : public QWidget, public Ui::Inspector
    {
        Q_OBJECT

    public:

        CInspector(QWidget* _pParent = Q_NULLPTR);
        ~CInspector();

    public Q_SLOTS:

        void updateContentForEntity(int _ID);
        void updateContentForTexture(const QString& _rRelPath);
        void updateContentForMaterial(const QString& _rRelPath);
        void updateContentForModel(const QString& _rRelPath);

    protected:

        void mousePressEvent(QMouseEvent* _pEvent);

    private:

        QVBoxLayout*              m_pInspectorLayout;
        CInspectorEntity*         m_pEntityWidget;
        CInspectorPointlight*     m_pPointlightWidget;
        CInspectorArealight*      m_pArealightWidget;
        CInspectorSun*            m_pSunWidget;
        CInspectorTransformation* m_pTransformWidget;
        CInspectorEnvironment*    m_pEnvironmentWidget;
        CInspectorLightProbe*     m_pLightProbeWidget;
        CInspectorBloom*          m_pBloomWidget;
        CInspectorDOF*            m_pDOFWidget;
        CInspectorPostAA*         m_pPostAAWidget;
        CInspectorSSR*            m_pSSRWidget;
        CInspectorVolumeFog*      m_pVolumeFogWidget;
        CInspectorMaterial*       m_pMaterialWidget;
        CInspectorCamera*         m_pCameraWidget;
        CInspectorARController*   m_pARControllerWidget;
        CInspectorTexture*        m_pTextureWidget;
        CInspectorSLAM*           m_pSlamWidget;
        unsigned int              m_ActiveEntityID;

    private:

        void OnEntityInfoFacets(Edit::CMessage& _rMessage);

        void ResetLayout();

        void HighlightEntity(int _ID);
        void ResetHighlight();
    };
} // namespace Edit