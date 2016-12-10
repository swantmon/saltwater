
#include "base/base_crc.h"
#include "base/base_vector3.h"
#include "base/base_vector4.h"

#include "editor_gui/edit_inspector_material.h"

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

        Base::Float3 AlbedoColor = Base::Float3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        QString    NewColorTexture = m_pAlbedoTextureEdit->text();
        QByteArray NewColorTextureBinary = NewColorTexture.toLatin1();

        // -----------------------------------------------------------------------------

        QString    NewNormalTexture = m_pNormalTextureEdit->text();
        QByteArray NewNormalTextureBinary = NewNormalTexture.toLatin1();

        // -----------------------------------------------------------------------------

        QString    NewRoughnessTexture = m_pRoughnessTextureEdit->text();
        QByteArray NewRoughnessTextureBinary = NewRoughnessTexture.toLatin1();

        float RoughnessValue = m_pRoughnessEdit  ->text().toFloat();

        // -----------------------------------------------------------------------------

        QString    NewMetalicTexture = m_pMetallicTextureEdit->text();
        QByteArray NewMetalicTextureBinary = NewMetalicTexture.toLatin1();

        float MetallicValue = m_pMetallicEdit->text().toFloat();

        // -----------------------------------------------------------------------------

        float ReflectanceValue = m_pReflectanceEdit->text().toFloat();

        // -----------------------------------------------------------------------------

        QString    NewBumpTexture = m_pBumpTextureEdit->text();
        QByteArray NewBumpTextureBinary = NewBumpTexture.toLatin1();

        // -----------------------------------------------------------------------------

        QString    NewAOTexture = m_pAOTextureEdit->text();
        QByteArray NewAOTextureBinary = NewAOTexture.toLatin1();

        // -----------------------------------------------------------------------------

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

        if (NewColorTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewColorTextureBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewNormalTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewNormalTextureBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewRoughnessTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewRoughnessTextureBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewMetalicTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewMetalicTextureBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewBumpTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewBumpTextureBinary.data());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (NewAOTextureBinary.length() > 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutString(NewAOTextureBinary.data());
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
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Actor_Material_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::dragEnterEvent(QDragEnterEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasText())
        {
            QString Text = pMimeData->text();

            QFileInfo FileInfo(Text);

            if (FileInfo.completeSuffix() == "mat")
            {
                _pEvent->acceptProposedAction();
            }
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorMaterial::dropEvent(QDropEvent* _pEvent)
    {
        const QMimeData* pMimeData = _pEvent->mimeData();

        if (pMimeData->hasUrls())
        {
            QUrl Url = pMimeData->urls()[0];

            QFileInfo FileInfo(Url.toLocalFile());

            if (FileInfo.completeSuffix() == "mat")
            {
                QDir Directory("../assets/");

                QString AbsPath = FileInfo.absoluteFilePath();

                QByteArray ModelFileBinary = Directory.relativeFilePath(AbsPath).toLatin1();

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
        }
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

        Base::Float3 AlbedoColor = Base::Float3(R, G, B);

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();
        W = _rMessage.GetFloat();

        Base::Float4 TilingOffset = Base::Float4(X, Y, Z, W);

        float Roughness   = _rMessage.GetFloat();
        float Reflectance = _rMessage.GetFloat();
        float Metalness   = _rMessage.GetFloat();

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

        m_pAlbedoTextureEdit->setText("");

        if (HasColorMap) m_pAlbedoTextureEdit->setText(ColorMapName);

        QPalette ButtonPalette = m_pAlbedoColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(AlbedoColor[0] * 255.0f, AlbedoColor[1] * 255.0f, AlbedoColor[2] * 255.0f));

        m_pAlbedoColorButton->setPalette(ButtonPalette);

        m_pAlbedoColorButton->update();

        // -----------------------------------------------------------------------------

        m_pNormalTextureEdit->setText("");

        if (HasNormalMap) m_pNormalTextureEdit->setText(NormalMapName);

        // -----------------------------------------------------------------------------

        m_pRoughnessTextureEdit->setText("");

        if (HasRoughnessMap) m_pRoughnessTextureEdit->setText(RoughnessMapName);

        m_pRoughnessSlider->setValue(static_cast<int>(Roughness * 100.0f));

        m_pRoughnessEdit->setText(QString::number(Roughness));

        // -----------------------------------------------------------------------------

        m_pMetallicTextureEdit->setText("");

        if (HasMetalnessMap) m_pMetallicTextureEdit->setText(MetalMapName);

        m_pMetallicSlider->setValue(static_cast<int>(Metalness * 100.0f));

        m_pMetallicEdit->setText(QString::number(Metalness));

        // -----------------------------------------------------------------------------

        m_pReflectanceSlider->setValue(static_cast<int>(Reflectance * 100.0f));

        m_pReflectanceEdit->setText(QString::number(Reflectance));

        // -----------------------------------------------------------------------------

        m_pBumpTextureEdit->setText("");

        if (HasBumpMap) m_pBumpTextureEdit->setText(BumpMapName);

        // -----------------------------------------------------------------------------

        m_pAOTextureEdit->setText("");

        if (HasAOMap) m_pAOTextureEdit->setText(AOMapName);

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