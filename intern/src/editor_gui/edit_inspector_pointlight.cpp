
#include "base/base_include_glm.h"

#include "editor_gui/edit_inspector_pointlight.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorPointlight::CInspectorPointlight(QWidget* _pParent)
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
        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Light_Pointlight_Info, EDIT_RECEIVE_MESSAGE(&CInspectorPointlight::OnEntityInfoPointlight));
    }

    // -----------------------------------------------------------------------------

    CInspectorPointlight::~CInspectorPointlight()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int ColorMode = m_pColorModeCB->currentIndex();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        glm::vec3 Color = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        float Temperature       = m_pTemperatureEdit->text().toFloat();
        float Intensity         = m_pIntensityEdit->text().toFloat();
        float AttenuationRadius = m_pAttenuationRadiusEdit->text().toFloat();
        float InnerConeAngle    = m_pInnerConeAngleEdit->text().toFloat();
        float OuterConeAngle    = m_pOuterConeAngleEdit->text().toFloat();

        glm::vec3 Direction = glm::vec3(m_pDirectionXEdit->text().toFloat(), m_pDirectionYEdit->text().toFloat(), m_pDirectionZEdit->text().toFloat());

        int ShadowType    = m_pShadowTypeCB->currentIndex();
        int ShadowQuality = m_pShadowQualityCB->currentIndex();
        int ShadowRefresh = m_pShadowRefreshCB->currentIndex();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(ColorMode);

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);

        NewMessage.PutFloat(Temperature);
        NewMessage.PutFloat(Intensity);
        NewMessage.PutFloat(AttenuationRadius);
        NewMessage.PutFloat(InnerConeAngle);
        NewMessage.PutFloat(OuterConeAngle);

        NewMessage.PutFloat(Direction[0]);
        NewMessage.PutFloat(Direction[1]);
        NewMessage.PutFloat(Direction[2]);

        NewMessage.PutInt(ShadowType);
        NewMessage.PutInt(ShadowQuality);
        NewMessage.PutInt(ShadowRefresh);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Pointlight_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Pointlight_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::OnEntityInfoPointlight(Edit::CMessage& _rMessage)
    {
        float R, G, B;
        float X, Y, Z;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID  = _rMessage.GetInt();
        int ColorMode = _rMessage.GetInt();

        BASE_UNUSED(EntityID);

        R = _rMessage.GetFloat();
        G = _rMessage.GetFloat();
        B = _rMessage.GetFloat();

        glm::ivec3 Color = glm::ivec3(R * 255, G * 255, B * 255);

        float Temperature       = _rMessage.GetFloat();
        float Intensity         = _rMessage.GetFloat();
        float AttenuationRadius = _rMessage.GetFloat();
        float InnerConeAngle    = _rMessage.GetFloat();
        float OuterConeAngle    = _rMessage.GetFloat();

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();

        glm::vec3 Direction = glm::vec3(X, Y, Z);

        int ShadowType    = _rMessage.GetInt();
        int ShadowQuality = _rMessage.GetInt();
        int ShadowRefresh = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pColorModeCB    ->blockSignals(true);
        m_pShadowTypeCB   ->blockSignals(true);
        m_pShadowQualityCB->blockSignals(true);
        m_pShadowRefreshCB->blockSignals(true);

        m_pColorModeCB->setCurrentIndex(ColorMode);

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[0], Color[1], Color[2]));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        m_pTemperatureEdit      ->setText(QString::number(Temperature));
        m_pIntensityEdit        ->setText(QString::number(Intensity));
        m_pAttenuationRadiusEdit->setText(QString::number(AttenuationRadius));
        m_pInnerConeAngleEdit   ->setText(QString::number(InnerConeAngle));
        m_pOuterConeAngleEdit   ->setText(QString::number(OuterConeAngle));

        m_pDirectionXEdit->setText(QString::number(Direction[0]));
        m_pDirectionYEdit->setText(QString::number(Direction[1]));
        m_pDirectionZEdit->setText(QString::number(Direction[2]));

        m_pShadowTypeCB   ->setCurrentIndex(ShadowType);
        m_pShadowQualityCB->setCurrentIndex(ShadowQuality);
        m_pShadowRefreshCB->setCurrentIndex(ShadowRefresh);

        m_pColorModeCB    ->blockSignals(false);
        m_pShadowTypeCB   ->blockSignals(false);
        m_pShadowQualityCB->blockSignals(false);
        m_pShadowRefreshCB->blockSignals(false);
    }
} // namespace Edit
