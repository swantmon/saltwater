
#include "base/base_include_glm.h"

#include "editor_gui/edit_inspector_volumefog.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorVolumeFog::CInspectorVolumeFog(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
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

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(WindDirection[0]);
        NewMessage.Put(WindDirection[1]);
        NewMessage.Put(WindDirection[2]);
        NewMessage.Put(WindDirection[3]);

        NewMessage.Put(Color[0]);
        NewMessage.Put(Color[1]);
        NewMessage.Put(Color[2]);
        NewMessage.Put(Color[3]);

        NewMessage.Put(FrustumDepth);
        NewMessage.Put(ShadowIntensity);
        NewMessage.Put(ScatteringCoeff);
        NewMessage.Put(AbsorptionCoeff);
        NewMessage.Put(DensityLevel);
        NewMessage.Put(DensityAttenuation);

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

    void CInspectorVolumeFog::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

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
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        X = _rMessage.Get<float>();
        Y = _rMessage.Get<float>();
        Z = _rMessage.Get<float>();
        W = _rMessage.Get<float>();

        glm::vec4 Direction = glm::vec4(X, Y, Z, W);

        R = _rMessage.Get<float>();
        G = _rMessage.Get<float>();
        B = _rMessage.Get<float>();
        A = _rMessage.Get<float>();
        
        glm::ivec4 Color = glm::ivec4(R * 255, G * 255, B * 255, A * 255);

        float FrustumDepth       = _rMessage.Get<float>();
        float ShadowIntensity    = _rMessage.Get<float>();
        float ScatteringCoeff    = _rMessage.Get<float>();
        float AbsorptionCoeff    = _rMessage.Get<float>();
        float DensityLevel       = _rMessage.Get<float>();
        float DensityAttenuation = _rMessage.Get<float>();

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
