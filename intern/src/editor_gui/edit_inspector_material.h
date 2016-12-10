#pragma once

#include "editor_gui/ui_edit_inspector_material.h"

#include "editor_port/edit_message.h"

#include <QWidget>

namespace Edit
{
    class CInspectorMaterial : public QWidget, public Ui::InspectorMaterial
    {
        Q_OBJECT

    public:

        CInspectorMaterial(QWidget* _pParent = Q_NULLPTR);
        ~CInspectorMaterial();

    public Q_SLOTS:

        void valueChanged();

        void roughnessValueChanged(int _Value);

        void metallicValueChanged(int _Value);

        void reflectanceValueChanged(int _Value);

        void pickColorFromDialog();

        void RequestInformation(unsigned int _EntityID);

    private:

        unsigned int m_CurrentEntityID;
        unsigned int m_MaterialHash;

    private:

        void dragEnterEvent(QDragEnterEvent* _pEvent);
        void dropEvent(QDropEvent* _pEvent);

    private:

        void OnEntityInfoMaterial(Edit::CMessage& _rMessage);
        void OnMaterialInfo(Edit::CMessage& _rMessage);
    };
} // namespace Edit

