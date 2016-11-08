#pragma once

#include "editor_gui/ui_edit_inspector_transformation.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QWidget>

namespace Edit
{
    class CInspectorTransformation : public QWidget, public Ui::InspectorTransformation
    {
	    Q_OBJECT

    public:

	    CInspectorTransformation(QWidget* _pParent = Q_NULLPTR);
	    ~CInspectorTransformation();

    public Q_SLOTS:

        void valueChanged();

        void changePositionX(QPoint _Difference);
        void changePositionY(QPoint _Difference);
        void changePositionZ(QPoint _Difference);

        void changeRotationX(QPoint _Difference);
        void changeRotationY(QPoint _Difference);
        void changeRotationZ(QPoint _Difference);

        void changeScaleX(QPoint _Difference);
        void changeScaleY(QPoint _Difference);
        void changeScaleZ(QPoint _Difference);

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;

    private:

        void OnEntityInfoTransformation(Edit::CMessage& _rMessage);
    };
} // namespace Edit