
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

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(ColorMode);

        NewMessage.Put(Color);

        NewMessage.Put(Temperature);
        NewMessage.Put(Intensity);
        NewMessage.Put(AttenuationRadius);
        NewMessage.Put(InnerConeAngle);
        NewMessage.Put(OuterConeAngle);

        NewMessage.Put(Direction);

        NewMessage.Put(ShadowType);
        NewMessage.Put(ShadowQuality);
        NewMessage.Put(ShadowRefresh);

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

    void CInspectorPointlight::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Pointlight_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorPointlight::OnEntityInfoPointlight(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID  = _rMessage.Get<Base::ID>();
        int ColorMode = _rMessage.Get<int>();

        BASE_UNUSED(EntityID);

        glm::vec3 MsgColor = _rMessage.Get<glm::vec3>();

        glm::ivec3 Color = glm::ivec3(MsgColor.r * 255, MsgColor.g * 255, MsgColor.b * 255);

        float Temperature       = _rMessage.Get<float>();
        float Intensity         = _rMessage.Get<float>();
        float AttenuationRadius = _rMessage.Get<float>();
        float InnerConeAngle    = _rMessage.Get<float>();
        float OuterConeAngle    = _rMessage.Get<float>();

        glm::vec3 Direction = _rMessage.Get<glm::vec3>();

        int ShadowType    = _rMessage.Get<int>();
        int ShadowQuality = _rMessage.Get<int>();
        int ShadowRefresh = _rMessage.Get<int>();

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
