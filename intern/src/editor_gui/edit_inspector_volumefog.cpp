
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_volumefog.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorVolumeFog::CInspectorVolumeFog(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pFogColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pFogColorButton->setPalette(ButtonPalette);

        m_pFogColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoVolumeFog, EDIT_RECEIVE_MESSAGE(&CInspectorVolumeFog::OnEntityInfoVolumeFog));
    }

    // -----------------------------------------------------------------------------

    CInspectorVolumeFog::~CInspectorVolumeFog()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::Float3 WindDirection = Base::Float3(m_pWindDirectionXEdit->text().toFloat(), m_pWindDirectionYEdit->text().toFloat(), m_pWindDirectionZEdit->text().toFloat());

        QPalette ButtonPalette = m_pFogColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 Color = Base::Float3(RGB.blue() / 255.0f, RGB.green() / 255.0f, RGB.red() / 255.0f);

        float FrustumDepth       = m_pFrustumDepthEdit      ->text().toFloat();
        float ShadowIntensity    = m_pShadowIntensityEdit   ->text().toFloat();
        float ScatteringCoeff    = m_pScatteringCoeffEdit   ->text().toFloat();
        float AbsorptionCoeff    = m_pAbsorptionCoeffEdit   ->text().toFloat();
        float DensityLevel       = m_pDensityLevelEdit      ->text().toFloat();
        float DensityAttenuation = m_pDensityAttenuationEdit->text().toFloat();


        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutFloat(WindDirection[0]);
        NewMessage.PutFloat(WindDirection[1]);
        NewMessage.PutFloat(WindDirection[2]);

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);

        NewMessage.PutFloat(FrustumDepth);
        NewMessage.PutFloat(ShadowIntensity);
        NewMessage.PutFloat(ScatteringCoeff);
        NewMessage.PutFloat(AbsorptionCoeff);
        NewMessage.PutFloat(DensityLevel);
        NewMessage.PutFloat(DensityAttenuation);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::EntityInfoVolumeFog, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pFogColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pFogColorButton->setPalette(ButtonPalette);

        m_pFogColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoVolumeFog, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::OnEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::Float3 Direction = Base::Float3(_rMessage.GetFloat(), _rMessage.GetFloat(), _rMessage.GetFloat());
        
        Base::Int3 Color = Base::Int3(_rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255, _rMessage.GetFloat() * 255);

        float FrustumDepth       = _rMessage.GetFloat();
        float ShadowIntensity    = _rMessage.GetFloat();
        float ScatteringCoeff    = _rMessage.GetFloat();
        float AbsorptionCoeff    = _rMessage.GetFloat();
        float DensityLevel       = _rMessage.GetFloat();
        float DensityAttenuation = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pWindDirectionXEdit->setText(QString::number(Direction[0]));
        m_pWindDirectionYEdit->setText(QString::number(Direction[1]));
        m_pWindDirectionZEdit->setText(QString::number(Direction[2]));

        QPalette ButtonPalette = m_pFogColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[2], Color[1], Color[0]));

        m_pFogColorButton->setPalette(ButtonPalette);

        m_pFogColorButton->update();

        m_pFrustumDepthEdit      ->setText(QString::number(FrustumDepth));
        m_pShadowIntensityEdit   ->setText(QString::number(ShadowIntensity));
        m_pScatteringCoeffEdit   ->setText(QString::number(ScatteringCoeff));
        m_pAbsorptionCoeffEdit   ->setText(QString::number(AbsorptionCoeff));
        m_pDensityLevelEdit      ->setText(QString::number(DensityLevel));
        m_pDensityAttenuationEdit->setText(QString::number(DensityAttenuation));
        
    }
} // namespace Edit
