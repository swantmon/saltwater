
#include "base/base_vector4.h"

#include "editor_gui/edit_inspector_bloom.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorBloom::CInspectorBloom(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(-1)
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EffectInfoBloom, EDIT_RECEIVE_MESSAGE(&CInspectorBloom::OnEntityInfoBloom));
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

        Base::Float4 Color = Base::Float4(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f, RGB.alpha() / 255.0f);

        float Intensity     = m_pIntensityEdit->text().toFloat();
        float Treshhold     = m_pTreshholdEdit->text().toFloat();
        float ExposureScale = m_pExposureScaleEdit->text().toFloat();
        int   Size          = m_pSizeEdit->text().toInt();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);
        NewMessage.PutFloat(Color[3]);

        NewMessage.PutFloat(Intensity);
        NewMessage.PutFloat(Treshhold);
        NewMessage.PutFloat(ExposureScale);
        NewMessage.PutInt(Size);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EffectInfoBloom, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::pickColorFromDialog()
    {
        QPalette ButtonPalette = m_pTintColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        QColor NewColor = QColorDialog::getColor(RGB);

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pTintColorButton->setPalette(ButtonPalette);

        m_pTintColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEffectInfoBloom, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorBloom::OnEntityInfoBloom(Edit::CMessage& _rMessage)
    {
        float R, G, B, A;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        R = _rMessage.GetFloat();
        G = _rMessage.GetFloat();
        B = _rMessage.GetFloat();
        A = _rMessage.GetFloat();

        Base::Int4 Color = Base::Int4(R * 255, G * 255, B * 255, A * 255);

        float Intensity     = _rMessage.GetFloat();
        float Treshhold     = _rMessage.GetFloat();
        float ExposureScale = _rMessage.GetFloat();
        int   Size          = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pTintColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[0], Color[1], Color[2], Color[3]));

        m_pTintColorButton->setPalette(ButtonPalette);

        m_pTintColorButton->update();

        m_pIntensityEdit    ->setText(QString::number(Intensity));
        m_pTreshholdEdit    ->setText(QString::number(Treshhold));
        m_pExposureScaleEdit->setText(QString::number(ExposureScale));
        m_pSizeEdit         ->setText(QString::number(Size));
    }
} // namespace Edit
