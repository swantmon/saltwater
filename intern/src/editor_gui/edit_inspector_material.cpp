
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
        , m_CurrentEntityID(-1)
        , m_MaterialHash   (0)
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

        float RoughnessValue   = m_pRoughnessEdit  ->text().toFloat();
        float MetallicValue    = m_pMetallicEdit->text().toFloat();
        float ReflectanceValue = m_pReflectanceEdit->text().toFloat();
        float BumpValue        = m_pBumpEdit->text().toFloat();

        float TilingX = m_pTilingXEdit->text().toFloat();
        float TilingY = m_pTilingYEdit->text().toFloat();
        float OffsetX = m_pOffsetXEdit->text().toFloat();
        float OffsetY = m_pOffsetYEdit->text().toFloat();

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

        NewMessage.PutInt(m_MaterialHash);

        NewMessage.PutFloat(AlbedoColor[0]);
        NewMessage.PutFloat(AlbedoColor[1]);
        NewMessage.PutFloat(AlbedoColor[2]);

        NewMessage.PutFloat(TilingX);
        NewMessage.PutFloat(TilingY);
        NewMessage.PutFloat(OffsetX);
        NewMessage.PutFloat(OffsetY);

        NewMessage.PutFloat(RoughnessValue);
        NewMessage.PutFloat(ReflectanceValue);
        NewMessage.PutFloat(MetallicValue);
        NewMessage.PutFloat(BumpValue);

        if (NewColorTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pAlbedoTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewNormalTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pNormalTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewRoughnessTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pRoughnessTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewMetalicTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pMetallicTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewBumpTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pBumpTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewAOTexture.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(m_pAOTextureEdit->GetTextureHash());
        }
        else
        {
            NewMessage.PutBool(false);
        }

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

    void CInspectorMaterial::RequestInformation(unsigned int _EntityID)
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

        NewMessage.PutInt(m_CurrentEntityID);

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
        Edit::CMessage NewMessage;

        NewMessage.PutString(_rRelPathToTexture.toLatin1().data());

        NewMessage.Reset();

        int Hash = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Load, NewMessage);

        if (Hash != -1)
        {
            m_MaterialHash = static_cast<unsigned int>(Hash);

            // -----------------------------------------------------------------------------
            // Request info of texture
            // -----------------------------------------------------------------------------
            Edit::CMessage NewMessage;

            NewMessage.PutInt(m_MaterialHash);

            NewMessage.Reset();

            Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Info, NewMessage);
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

        QByteArray ModelFileBinary = RelativePathToFile.toLatin1();

        CMessage NewLoadMaterialMessage;

        NewLoadMaterialMessage.PutString(ModelFileBinary.data());

        NewLoadMaterialMessage.Reset();

        int HashOfMaterial = Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Material_Load, NewLoadMaterialMessage);

        if (HashOfMaterial == -1) return;

        // -----------------------------------------------------------------------------
        // Set material to entity
        // -----------------------------------------------------------------------------
        Edit::CMessage NewApplyMessage;

        NewApplyMessage.PutInt(m_CurrentEntityID);

        NewApplyMessage.PutInt(HashOfMaterial);

        NewApplyMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Actor_Material_Update, NewApplyMessage);

        RequestInformation(m_CurrentEntityID);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::OnEntityInfoMaterial(Edit::CMessage& _rMessage)
    {
        float R, G, B;
        float X, Y, Z, W;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        bool HasMaterial = _rMessage.GetBool();

        if (HasMaterial == false)
        {
            return;
        }

        int MaterialHash = _rMessage.GetInt();

        m_MaterialHash = static_cast<unsigned int>(MaterialHash);

        CMessage NewMessage;

        NewMessage.PutInt(MaterialHash);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::OnMaterialInfo(Edit::CMessage& _rMessage)
    {
        float R, G, B;
        float X, Y, Z, W;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int MaterialHash = _rMessage.GetInt();

        if (MaterialHash != m_MaterialHash) return;

        bool HasColorMap     = false;
        bool HasNormalMap    = false;
        bool HasRoughnessMap = false;
        bool HasMetalnessMap = false;
        bool HasBumpMap      = false;
        bool HasAOMap        = false;

        char ColorMapName[256];
        char NormalMapName[256];
        char RoughnessMapName[256];
        char MetalMapName[256];
        char BumpMapName[256];
        char AOMapName[256];

        R = _rMessage.GetFloat();
        G = _rMessage.GetFloat();
        B = _rMessage.GetFloat();

        glm::vec3 AlbedoColor = glm::vec3(R, G, B);

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();
        W = _rMessage.GetFloat();

        glm::vec4 TilingOffset = glm::vec4(X, Y, Z, W);

        float Roughness   = _rMessage.GetFloat();
        float Reflectance = _rMessage.GetFloat();
        float Metalness   = _rMessage.GetFloat();
        float BumpFactor  = _rMessage.GetFloat();

        HasColorMap = _rMessage.GetBool();

        if (HasColorMap)
        {
            _rMessage.GetString(ColorMapName, 256);
        }

        HasNormalMap = _rMessage.GetBool();

        if (HasNormalMap)
        {
            _rMessage.GetString(NormalMapName, 256);
        }

        HasRoughnessMap = _rMessage.GetBool();

        if (HasRoughnessMap)
        {
            _rMessage.GetString(RoughnessMapName, 256);
        }

        HasMetalnessMap = _rMessage.GetBool();

        if (HasMetalnessMap)
        {
            _rMessage.GetString(MetalMapName, 256);
        }

        HasBumpMap = _rMessage.GetBool();

        if (HasBumpMap)
        {
            _rMessage.GetString(BumpMapName, 256);
        }

        HasAOMap = _rMessage.GetBool();

        if (HasAOMap)
        {
            _rMessage.GetString(AOMapName, 256);
        }

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

        m_pAlbedoTextureEdit->SetTextureFile("");

        if (HasColorMap) m_pAlbedoTextureEdit->SetTextureFile(ColorMapName);

        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(AlbedoColor[0] * 255.0f, AlbedoColor[1] * 255.0f, AlbedoColor[2] * 255.0f));

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        // -----------------------------------------------------------------------------

        m_pNormalTextureEdit->SetTextureFile("");

        if (HasNormalMap) m_pNormalTextureEdit->SetTextureFile(NormalMapName);

        // -----------------------------------------------------------------------------

        m_pRoughnessTextureEdit->SetTextureFile("");

        if (HasRoughnessMap) m_pRoughnessTextureEdit->SetTextureFile(RoughnessMapName);

        m_pRoughnessSlider->setValue(static_cast<int>(Roughness * 100.0f));

        m_pRoughnessEdit->setText(QString::number(Roughness));

        // -----------------------------------------------------------------------------

        m_pMetallicTextureEdit->SetTextureFile("");

        if (HasMetalnessMap) m_pMetallicTextureEdit->SetTextureFile(MetalMapName);

        m_pMetallicSlider->setValue(static_cast<int>(Metalness * 100.0f));

        m_pMetallicEdit->setText(QString::number(Metalness));

        // -----------------------------------------------------------------------------

        m_pReflectanceSlider->setValue(static_cast<int>(Reflectance * 100.0f));

        m_pReflectanceEdit->setText(QString::number(Reflectance));

        // -----------------------------------------------------------------------------

        m_pBumpTextureEdit->SetTextureFile("");

        if (HasBumpMap) m_pBumpTextureEdit->SetTextureFile(BumpMapName);

        m_pBumpEdit->setText(QString::number(BumpFactor));

        // -----------------------------------------------------------------------------

        m_pAOTextureEdit->SetTextureFile("");

        if (HasAOMap) m_pAOTextureEdit->SetTextureFile(AOMapName);

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