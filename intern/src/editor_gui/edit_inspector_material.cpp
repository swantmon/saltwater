
#include "base/base_crc.h"
#include "base/base_include_glm.h"

#include "editor_gui/edit_inspector_material.h"
#include "editor_gui/edit_texture_value.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMimeData>
#include <QResizeEvent>
#include <QUrl>

namespace Edit
{
    CInspectorMaterial::CInspectorMaterial(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
        , m_MaterialID     (0)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // User UI
        // -----------------------------------------------------------------------------
        m_pAlbedoTextureEdit   ->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));
        m_pNormalTextureEdit   ->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));
        m_pRoughnessTextureEdit->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));
        m_pMetallicTextureEdit ->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));
        m_pBumpTextureEdit     ->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));
        m_pAOTextureEdit       ->SetLayout((CTextureValue::NoPreview | CTextureValue::NoHash));

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pAlbedoTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pAlbedoTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        connect(m_pNormalTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pNormalTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        connect(m_pRoughnessTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pRoughnessTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        connect(m_pMetallicTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pMetallicTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        connect(m_pBumpTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pBumpTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

        connect(m_pAOTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));
        connect(m_pAOTextureEdit, SIGNAL(fileChanged(QString)), SLOT(valueChanged()));

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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Actor_Material_Info, EDIT_RECEIVE_MESSAGE(&CInspectorMaterial::OnEntityInfoMaterial));
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Material_Info      , EDIT_RECEIVE_MESSAGE(&CInspectorMaterial::OnMaterialInfo));
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

        glm::vec3 AlbedoColor = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        QString NewColorTexture     = m_pAlbedoTextureEdit->GetTextureFile();
        QString NewNormalTexture    = m_pNormalTextureEdit->GetTextureFile();
        QString NewRoughnessTexture = m_pRoughnessTextureEdit->GetTextureFile();
        QString NewMetalicTexture   = m_pMetallicTextureEdit->GetTextureFile();
        QString NewBumpTexture      = m_pBumpTextureEdit->GetTextureFile();
        QString NewAOTexture        = m_pAOTextureEdit->GetTextureFile();

        float RoughnessValue   = m_pRoughnessEdit->text().toFloat();
        float MetallicValue    = m_pMetallicEdit->text().toFloat();
        float ReflectanceValue = m_pReflectanceEdit->text().toFloat();
        float BumpValue        = m_pBumpEdit->text().toFloat();

        float TilingX = m_pTilingXEdit->text().toFloat();
        float TilingY = m_pTilingYEdit->text().toFloat();
        float OffsetX = m_pOffsetXEdit->text().toFloat();
        float OffsetY = m_pOffsetYEdit->text().toFloat();

        glm::vec4 TilingOffset = glm::vec4(TilingX, TilingY, OffsetX, OffsetY);

        // -----------------------------------------------------------------------------
        // Update related GUI
        // -----------------------------------------------------------------------------
        m_pRoughnessSlider  ->blockSignals(true);
        m_pMetallicSlider   ->blockSignals(true);
        m_pReflectanceSlider->blockSignals(true);

        m_pRoughnessSlider  ->setValue(static_cast<int>(RoughnessValue   * 100.0f));
        m_pMetallicSlider   ->setValue(static_cast<int>(MetallicValue    * 100.0f));
        m_pReflectanceSlider->setValue(static_cast<int>(ReflectanceValue * 100.0f));

        m_pRoughnessSlider  ->blockSignals(false);
        m_pMetallicSlider   ->blockSignals(false);
        m_pReflectanceSlider->blockSignals(false);


        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_MaterialID);

        NewMessage.Put(AlbedoColor);

        NewMessage.Put(TilingOffset);

        NewMessage.Put(RoughnessValue);
        NewMessage.Put(ReflectanceValue);
        NewMessage.Put(MetallicValue);
        NewMessage.Put(BumpValue);

        std::string AlbedoTextureEdit = std::string(m_pAlbedoTextureEdit->GetTextureFile().toLatin1());
        std::string NormalTextureEdit = std::string(m_pNormalTextureEdit->GetTextureFile().toLatin1());
        std::string RoughnessTextureEdit = std::string(m_pRoughnessTextureEdit->GetTextureFile().toLatin1());
        std::string MetallicTextureEdit = std::string(m_pMetallicTextureEdit->GetTextureFile().toLatin1());
        std::string BumpTextureEdit = std::string(m_pBumpTextureEdit->GetTextureFile().toLatin1());
        std::string AOTextureEdit = std::string(m_pAOTextureEdit->GetTextureFile().toLatin1());

        NewMessage.Put(AlbedoTextureEdit);
        NewMessage.Put(NormalTextureEdit);
        NewMessage.Put(RoughnessTextureEdit);
        NewMessage.Put(MetallicTextureEdit);
        NewMessage.Put(BumpTextureEdit);
        NewMessage.Put(AOTextureEdit);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Update, NewMessage);
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
        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        QColor NewColor = QColorDialog::getColor(RGB);

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::RequestInformation(Base::ID _EntityID)
    {
        // -----------------------------------------------------------------------------
        // Behavior
        // -----------------------------------------------------------------------------
        setAcceptDrops(true);

        // -----------------------------------------------------------------------------
        // Load material from entity
        // -----------------------------------------------------------------------------
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Actor_Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::RequestInformation(const QString& _rRelPathToTexture)
    {
        // -----------------------------------------------------------------------------
        // Behavior
        // -----------------------------------------------------------------------------
        setAcceptDrops(false);

        // -----------------------------------------------------------------------------
        // Load material from file
        // -----------------------------------------------------------------------------
        Edit::CMessage LoadMessage;

        LoadMessage.Put(std::string(_rRelPathToTexture.toLatin1()));

        LoadMessage.Reset();

        int Hash = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Load, LoadMessage);

        if (Hash != -1)
        {
            m_MaterialID = static_cast<Base::ID>(Hash);

            // -----------------------------------------------------------------------------
            // Request info of texture
            // -----------------------------------------------------------------------------
            Edit::CMessage RequestMessage;

            RequestMessage.Put(m_MaterialID);

            RequestMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Info, RequestMessage);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasFormat("SW_MATERIAL_REL_PATH") == false) return;

        _pEvent->acceptProposedAction();
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        assert(pMimeData->hasFormat("SW_MATERIAL_REL_PATH"));

        QString RelativePathToFile = pMimeData->data("SW_MATERIAL_REL_PATH");

        CMessage NewLoadMaterialMessage;

        NewLoadMaterialMessage.Put(std::string(RelativePathToFile.toLatin1()));

        NewLoadMaterialMessage.Reset();

        int Result = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Load, NewLoadMaterialMessage);

        if (Result == -1) return;

        // -----------------------------------------------------------------------------
        // Set material to entity
        // -----------------------------------------------------------------------------
        Edit::CMessage NewApplyMessage;

        NewApplyMessage.Put(m_CurrentEntityID);

        NewApplyMessage.Put(static_cast<Base::ID>(Result));

        NewApplyMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Actor_Material_Update, NewApplyMessage);

        RequestInformation(m_CurrentEntityID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::OnEntityInfoMaterial(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        bool HasMaterial = _rMessage.Get<bool>();

        if (HasMaterial == false)
        {
            return;
        }

        Base::ID MaterialID = _rMessage.Get<Base::ID>();

        m_MaterialID = static_cast<unsigned int>(MaterialID);

        CMessage NewMessage;

        NewMessage.Put(MaterialID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::OnMaterialInfo(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID MaterialID = _rMessage.Get<Base::ID>();

        if (MaterialID != m_MaterialID) return;

        glm::vec3 AlbedoColor = _rMessage.Get<glm::vec3>();

        glm::vec4 TilingOffset = _rMessage.Get<glm::vec4>();

        float Roughness   = _rMessage.Get<float>();
        float Reflectance = _rMessage.Get<float>();
        float Metalness   = _rMessage.Get<float>();
        float BumpFactor  = _rMessage.Get<float>();

        std::string ColorMapName     = _rMessage.Get<std::string>();
        std::string NormalMapName    = _rMessage.Get<std::string>();
        std::string RoughnessMapName = _rMessage.Get<std::string>();
        std::string MetalMapName     = _rMessage.Get<std::string>();
        std::string BumpMapName      = _rMessage.Get<std::string>();
        std::string AOMapName        = _rMessage.Get<std::string>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        
        m_pRoughnessEdit    ->blockSignals(true);
        m_pMetallicEdit     ->blockSignals(true);
        m_pReflectanceEdit  ->blockSignals(true);
        m_pRoughnessSlider  ->blockSignals(true);
        m_pMetallicSlider   ->blockSignals(true);
        m_pReflectanceSlider->blockSignals(true);

        // -----------------------------------------------------------------------------

        m_pAlbedoTextureEdit->SetTextureFile(QString(ColorMapName.c_str()));

        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(AlbedoColor[0] * 255.0f, AlbedoColor[1] * 255.0f, AlbedoColor[2] * 255.0f));

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        // -----------------------------------------------------------------------------

        m_pNormalTextureEdit->SetTextureFile(QString(NormalMapName.c_str()));

        // -----------------------------------------------------------------------------

        m_pRoughnessTextureEdit->SetTextureFile(QString(RoughnessMapName.c_str()));

        m_pRoughnessSlider->setValue(static_cast<int>(Roughness * 100.0f));

        m_pRoughnessEdit->setText(QString::number(Roughness));

        // -----------------------------------------------------------------------------

        m_pMetallicTextureEdit->SetTextureFile(QString(MetalMapName.c_str()));

        m_pMetallicSlider->setValue(static_cast<int>(Metalness * 100.0f));

        m_pMetallicEdit->setText(QString::number(Metalness));

        // -----------------------------------------------------------------------------

        m_pReflectanceSlider->setValue(static_cast<int>(Reflectance * 100.0f));

        m_pReflectanceEdit->setText(QString::number(Reflectance));

        // -----------------------------------------------------------------------------

        m_pBumpTextureEdit->SetTextureFile(QString(BumpMapName.c_str()));

        m_pBumpEdit->setText(QString::number(BumpFactor));

        // -----------------------------------------------------------------------------

        m_pAOTextureEdit->SetTextureFile(QString(AOMapName.c_str()));

        // -----------------------------------------------------------------------------

        m_pTilingXEdit->setText(QString::number(TilingOffset[0]));

        m_pTilingYEdit->setText(QString::number(TilingOffset[1]));

        m_pOffsetXEdit->setText(QString::number(TilingOffset[2]));

        m_pOffsetYEdit->setText(QString::number(TilingOffset[3]));

        // -----------------------------------------------------------------------------

        m_pRoughnessEdit    ->blockSignals(false);
        m_pMetallicEdit     ->blockSignals(false);
        m_pReflectanceEdit  ->blockSignals(false);
        m_pRoughnessSlider  ->blockSignals(false);
        m_pMetallicSlider   ->blockSignals(false);
        m_pReflectanceSlider->blockSignals(false);
    }
} // namespace Edit