
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_bloom.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorBloom::CInspectorBloom(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pTintColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pTintColorButton->setPalette(ButtonPalette);

        m_pTintColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoBloom, EDIT_RECEIVE_MESSAGE(&CInspectorBloom::OnEntityInfoBloom));
    }

    // -----------------------------------------------------------------------------

    CInspectorBloom::~CInspectorBloom()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pTintColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 Color = Base::Float3(RGB.blue() / 255.0f, RGB.green() / 255.0f, RGB.red() / 255.0f);

        float Intensity     = m_pIntensityEdit->text().toFloat();
        float Treshhold     = m_pTreshholdEdit->text().toFloat();
        float ExposureScale = m_pExposureScaleEdit->text().toFloat();
        int   Size          = m_pSizeEdit->text().toInt();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);

        NewMessage.PutFloat(Intensity);
        NewMessage.PutFloat(Treshhold);
        NewMessage.PutFloat(ExposureScale);
        NewMessage.PutInt(Size);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoBloom, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pTintColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pTintColorButton->setPalette(ButtonPalette);

        m_pTintColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoBloom, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::OnEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::Int3 Color = Base::Int3(_rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255);

        float Intensity     = _rMessage.GetFloat();
        float Treshhold     = _rMessage.GetFloat();
        float ExposureScale = _rMessage.GetFloat();
        int   Size          = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pTintColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[2], Color[1], Color[0]));

        m_pTintColorButton->setPalette(ButtonPalette);

        m_pTintColorButton->update();

        m_pIntensityEdit    ->setText(QString::number(Intensity));
        m_pTreshholdEdit    ->setText(QString::number(Treshhold));
        m_pExposureScaleEdit->setText(QString::number(ExposureScale));
        m_pSizeEdit         ->setText(QString::number(Size));
    }
} // namespace Edit
