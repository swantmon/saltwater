
#include "base/base_include_glm.h"

#include "editor_gui/edit_inspector_arealight.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorArealight::CInspectorArealight(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<Base::ID>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // User UI
        // -----------------------------------------------------------------------------
        m_pTextureEdit->SetLayout((CTextureValue::NoPreview));

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Light_Arealight_Info, EDIT_RECEIVE_MESSAGE(&CInspectorArealight::OnEntityInfoArealight));
    }

    // -----------------------------------------------------------------------------

    CInspectorArealight::~CInspectorArealight()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorArealight::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int ColorMode = m_pColorModeCB->currentIndex();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        glm::vec3 Color = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        QString NewTexture = m_pTextureEdit->GetTextureFile();

        float Temperature = m_pTemperatureEdit->text().toFloat();
        float Intensity   = m_pIntensityEdit->text().toFloat();
        float Rotation    = m_pRotationEdit->text().toFloat();
        float Width       = m_pWidthEdit->text().toFloat();
        float Height      = m_pHeightEdit->text().toFloat();
        bool  IsTwoSided  = m_IsTwoSidedCB->isChecked();

        glm::vec3 Direction = glm::vec3(m_pDirectionXEdit->text().toFloat(), m_pDirectionYEdit->text().toFloat(), m_pDirectionZEdit->text().toFloat());

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(ColorMode);

        NewMessage.Put(Color);

        NewMessage.Put(Temperature);
        NewMessage.Put(Intensity);
        NewMessage.Put(Rotation);
        NewMessage.Put(Width);
        NewMessage.Put(Height);
        NewMessage.Put(IsTwoSided);

        NewMessage.Put(Direction[0]);
        NewMessage.Put(Direction[1]);
        NewMessage.Put(Direction[2]);

        if (NewTexture.length() > 0)
        {
            NewMessage.Put(true);

            NewMessage.Put(std::string(m_pTextureEdit->GetTextureFile().toLatin1()));
        }
        else
        {
            NewMessage.Put(false);
        }

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Light_Arealight_Update, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorArealight::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorArealight::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Light_Arealight_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorArealight::OnEntityInfoArealight(Edit::CMessage& _rMessage)
    {
        bool HasTexture = false;
        std::string TextureName;
        unsigned int TextureHash = 0;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID  = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        int ColorMode = _rMessage.Get<int>();

        glm::vec3 MsgColor = _rMessage.Get<glm::vec3>();

        glm::ivec3 Color = glm::ivec3(MsgColor.r * 255, MsgColor.g * 255, MsgColor.b * 255);

        float Temperature = _rMessage.Get<float>();
        float Intensity   = _rMessage.Get<float>();
        float Rotation    = _rMessage.Get<float>();
        float Width       = _rMessage.Get<float>();
        float Height      = _rMessage.Get<float>();
        bool  IsTwoSided  = _rMessage.Get<bool>();

        glm::vec3 Direction = _rMessage.Get<glm::vec3>();

        HasTexture = _rMessage.Get<bool>();

        if (HasTexture)
        {
            TextureName = _rMessage.Get<std::string>();

            TextureHash = _rMessage.Get<int>();
        }

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pColorModeCB    ->blockSignals(true);
        m_IsTwoSidedCB    ->blockSignals(true);

        m_pColorModeCB->setCurrentIndex(ColorMode);

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color.x, Color.y, Color.z));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        m_pTemperatureEdit->setText(QString::number(Temperature));
        m_pIntensityEdit  ->setText(QString::number(Intensity));
        m_pRotationEdit   ->setText(QString::number(Rotation));
        m_pWidthEdit      ->setText(QString::number(Width));
        m_pHeightEdit     ->setText(QString::number(Height));
        m_IsTwoSidedCB    ->setChecked(IsTwoSided);

        m_pDirectionXEdit->setText(QString::number(Direction[0]));
        m_pDirectionYEdit->setText(QString::number(Direction[1]));
        m_pDirectionZEdit->setText(QString::number(Direction[2]));

        m_pTextureEdit->SetTextureFile("");

        if (HasTexture)
        {
            m_pTextureEdit->SetTextureFile(QString(TextureName.c_str()));
        }

        m_pColorModeCB    ->blockSignals(false);
        m_IsTwoSidedCB    ->blockSignals(false);
    }
} // namespace Edit
