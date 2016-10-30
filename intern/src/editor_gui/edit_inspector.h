#pragma once

#include "editor_gui/edit_inspector_environment.h"
#include "editor_gui/edit_inspector_globalprobe.h"
#include "editor_gui/edit_inspector_pointlight.h"
#include "editor_gui/edit_inspector_transformation.h"

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
        CInspectorPointlight*     m_pPointlightWidget;
        CInspectorTransformation* m_pTransformWidget;
        CInspectorEnvironment*    m_pEnvironmentWidget;
        CInspectorGlobalProbe*    m_pGlobalProbeWidget;

    private:

        void OnEntityInfoFacets(Edit::CMessage& _rMessage);
    };
} // namespace Edit