#pragma once

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
        CInspectorTransformation* m_pTransformWidget;

    private:

        void OnEntityInfoFacets(Edit::CMessage& _rMessage);
    };
} // namespace Edit