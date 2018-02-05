
#include "editor_gui/edit_inspector_volumefog.h"

#include "editor_port/edit_message_manager.h"

#include "glm.hpp"

#include <QColorDialog>

namespace Edit
{
    CInspectorVolumeFog::CInspectorVolumeFog(QWidget* _pParent)
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
        QPalette ButtonPalette = m_pFogColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pFogColorButton->setPalette(ButtonPalette);

        m_pFogColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Effect_VolumeFog_Info, EDIT_RECEIVE_MESSAGE(&CInspectorVolumeFog::OnEntityInfoVolumeFog));
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
        glm::vec4 WindDirection = glm::vec4(m_pWindDirectionXEdit->text().toFloat(), m_pWindDirectionYEdit->text().toFloat(), m_pWindDirectionZEdit->text().toFloat(), 0.0f);

        QPalette ButtonPalette = m_pFogColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        glm::vec4 Color = glm::vec4(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f, RGB.alpha() / 255.0f);

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

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutFloat(WindDirection[0]);
        NewMessage.PutFloat(WindDirection[1]);
        NewMessage.PutFloat(WindDirection[2]);
        NewMessage.PutFloat(WindDirection[3]);

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);
        NewMessage.PutFloat(Color[3]);

        NewMessage.PutFloat(FrustumDepth);
        NewMessage.PutFloat(ShadowIntensity);
        NewMessage.PutFloat(ScatteringCoeff);
        NewMessage.PutFloat(AbsorptionCoeff);
        NewMessage.PutFloat(DensityLevel);
        NewMessage.PutFloat(DensityAttenuation);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Effect_VolumeFog_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::pickColorFromDialog()
    {
        QPalette ButtonPalette = m_pFogColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        QColor NewColor = QColorDialog::getColor(RGB);

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pFogColorButton->setPalette(ButtonPalette);

        m_pFogColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Effect_VolumeFog_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorVolumeFog::OnEntityInfoVolumeFog(Edit::CMessage& _rMessage)
    {
        float X, Y, Z, W;
        float R, G, B, A;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();
        W = _rMessage.GetFloat();

        glm::vec4 Direction = glm::vec4(X, Y, Z, W);

        R = _rMessage.GetFloat();
        G = _rMessage.GetFloat();
        B = _rMessage.GetFloat();
        A = _rMessage.GetFloat();
        
        glm::ivec4 Color = glm::ivec4(R * 255, G * 255, B * 255, A * 255);

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

        ButtonPalette.setColor(QPalette::Button, QColor(Color[0], Color[1], Color[2], Color[3]));

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
