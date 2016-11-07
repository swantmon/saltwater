
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_material.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorMaterial::CInspectorMaterial(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoMaterial, EDIT_RECEIVE_MESSAGE(&CInspectorMaterial::OnEntityInfoMaterial));
    }

    // -----------------------------------------------------------------------------

    CInspectorMaterial::~CInspectorMaterial() 
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 Color = Base::Float3(RGB.blue() / 255.0f, RGB.green() / 255.0f, RGB.red() / 255.0f);

        float RoughnessValue   = m_pRoughnessEdit->text().toFloat();
        float MetallicValue    = m_pMetallicEdit->text().toFloat();
        float ReflectanceValue = m_pReflectanceEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Update related GUI
        // -----------------------------------------------------------------------------
        m_pRoughnessSlider  ->blockSignals(true);
        m_pMetallicSlider   ->blockSignals(true);
        m_pReflectanceSlider->blockSignals(true);

        m_pRoughnessSlider  ->setValue(RoughnessValue * 100.0f);
        m_pMetallicSlider   ->setValue(MetallicValue * 100.0f);
        m_pReflectanceSlider->setValue(ReflectanceValue * 100.0f);

        m_pRoughnessSlider  ->blockSignals(false);
        m_pMetallicSlider   ->blockSignals(false);
        m_pReflectanceSlider->blockSignals(false);


        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoMaterial, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::roughnessValueChanged(int _Value)
    {
        m_pRoughnessEdit->setText(QString::number(_Value / 100.0f));
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::metallicValueChanged(int _Value)
    {
        m_pMetallicEdit->setText(QString::number(_Value / 100.0f));
    }

    // -----------------------------------------------------------------------------


    void CInspectorMaterial::reflectanceValueChanged(int _Value)
    {
        m_pReflectanceEdit->setText(QString::number(_Value / 100.0f));
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoMaterial, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::OnEntityInfoMaterial(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
    }
} // namespace Edit