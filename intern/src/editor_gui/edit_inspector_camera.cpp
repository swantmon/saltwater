
#include "editor_gui/edit_inspector_camera.h"

#include "editor_port/edit_message_manager.h"

#include "glm.hpp"

#include <QColorDialog>

namespace Edit
{
    CInspectorCamera::CInspectorCamera(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Setup user UI
        // -----------------------------------------------------------------------------
        m_pBackgroundTextureEdit->SetLayout(CTextureValue::NoPreview);

        // -----------------------------------------------------------------------------
        // Signal / slots
        // -----------------------------------------------------------------------------
        connect(m_pBackgroundTextureEdit, SIGNAL(hashChanged(unsigned int)), SLOT(valueChanged()));

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pSolidColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pSolidColorButton->setPalette(ButtonPalette);

        m_pSolidColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Actor_Camera_Info, EDIT_RECEIVE_MESSAGE(&CInspectorCamera::OnEntityInfoCamera));
    }

    // -----------------------------------------------------------------------------

    CInspectorCamera::~CInspectorCamera() 
    {

    }
    // -----------------------------------------------------------------------------

    void CInspectorCamera::valueChanged()
    {
        float X, Y, W, H;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        bool IsMainCamera = m_pIsMainCameraCB->isChecked();

        int ClearFlag = m_pClearFlagCS->currentIndex();

        int Hash = m_pBackgroundTextureEdit->GetTextureHash();

        QPalette ButtonPalette = m_pSolidColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        glm::vec3 AlbedoColor = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        int CullingMask = 0;

        int ProjectionType = m_pProjectionCB->currentIndex();

        float Size = m_pOrthographicSizeEdit->text().toFloat();

        float FOV = m_pFieldOfViewEdit->text().toFloat();

        float Near = m_pClippingPlaneNearEdit->text().toFloat();

        float Far = m_pClippingPlaneFarEdit->text().toFloat();

        X = m_pViewportXEdit->text().toFloat();
        Y = m_pViewportYEdit->text().toFloat();
        W = m_pViewportWEdit->text().toFloat();
        H = m_pViewportHEdit->text().toFloat();

        float Depth = m_pClearDepthEdit->text().toFloat();

        int CameraMode = m_pCameraModeCB->currentIndex();

        float ShutterSpeed = m_pShutterSpeedEdit->text().toFloat();

        float Aperture = m_pApertureEdit->text().toFloat();

        float ISO = m_pISOEdit->text().toFloat();

        float EC = m_pECEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Update related GUI
        // -----------------------------------------------------------------------------
        m_pFieldOfViewSlider->blockSignals(true);

        m_pFieldOfViewSlider->setValue(static_cast<int>(FOV));

        m_pFieldOfViewSlider->blockSignals(false);

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutBool(IsMainCamera);

        NewMessage.PutInt(ClearFlag);

        if (Hash != 0)
        {
            NewMessage.PutBool(true);

            NewMessage.PutInt(Hash);
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.PutFloat(AlbedoColor[0]);
        NewMessage.PutFloat(AlbedoColor[1]);
        NewMessage.PutFloat(AlbedoColor[2]);

        NewMessage.PutInt(CullingMask);

        NewMessage.PutInt(ProjectionType);

        NewMessage.PutFloat(Size);

        NewMessage.PutFloat(FOV);

        NewMessage.PutFloat(Near);

        NewMessage.PutFloat(Far);

        NewMessage.PutFloat(X);
        NewMessage.PutFloat(Y);
        NewMessage.PutFloat(W);
        NewMessage.PutFloat(H);

        NewMessage.PutFloat(Depth);

        NewMessage.PutInt(CameraMode);

        NewMessage.PutFloat(ShutterSpeed);

        NewMessage.PutFloat(Aperture);

        NewMessage.PutFloat(ISO);

        NewMessage.PutFloat(EC);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Actor_Camera_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorCamera::fieldOfViewValueChanged(int _Value)
    {
        m_pFieldOfViewEdit->setText(QString::number(_Value));
    }

    // -----------------------------------------------------------------------------

    void CInspectorCamera::pickColorFromDialog()
    {
        QPalette ButtonPalette = m_pSolidColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        QColor NewColor = QColorDialog::getColor(RGB);

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pSolidColorButton->setPalette(ButtonPalette);

        m_pSolidColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorCamera::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Actor_Camera_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorCamera::OnEntityInfoCamera(Edit::CMessage& _rMessage)
    {
        float R, G, B;
        float X, Y, W, H;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        // -----------------------------------------------------------------------------
        // Get values
        // -----------------------------------------------------------------------------
        bool IsMainCamera = _rMessage.GetBool();

        int ClearFlag = _rMessage.GetInt();

        bool HasTexture = _rMessage.GetBool();

        int TextureHash = -1;

        if (HasTexture)
        {
            TextureHash = _rMessage.GetInt();
        }

        R = _rMessage.GetFloat();
        G = _rMessage.GetFloat();
        B = _rMessage.GetFloat();

        int CullingMask = _rMessage.GetInt();

        int ProjectionType = _rMessage.GetInt();

        float Size = _rMessage.GetFloat();

        float FOV = _rMessage.GetFloat();

        float Near = _rMessage.GetFloat();

        float Far = _rMessage.GetFloat();

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        W = _rMessage.GetFloat();
        H = _rMessage.GetFloat();

        float Depth = _rMessage.GetFloat();

        int CameraMode = _rMessage.GetInt();

        float ShutterSpeed = _rMessage.GetFloat();

        float Aperture = _rMessage.GetFloat();

        float ISO = _rMessage.GetFloat();

        float EC = _rMessage.GetFloat();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pFieldOfViewSlider->blockSignals(true);
        m_pFieldOfViewEdit  ->blockSignals(true);
        m_pClearFlagCS      ->blockSignals(true);
        m_pProjectionCB     ->blockSignals(true);
        m_pCameraModeCB     ->blockSignals(true);

        m_pIsMainCameraCB->setChecked(IsMainCamera);

        m_pClearFlagCS->setCurrentIndex(ClearFlag);

        if (HasTexture)
        {
            m_pBackgroundTextureEdit->SetTextureHash(TextureHash);
        }
        else
        {
            m_pBackgroundTextureEdit->SetTextureHash(0);
        }

        QPalette ButtonPalette = m_pSolidColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(R * 255.0f, G * 255.0f, B * 255.0f));

        m_pSolidColorButton->setPalette(ButtonPalette);

        m_pSolidColorButton->update();

        m_pProjectionCB->setCurrentIndex(ProjectionType);

        m_pFieldOfViewSlider->setValue(FOV);

        m_pFieldOfViewEdit->setText(QString::number(FOV));

        m_pOrthographicSizeEdit->setText(QString::number(Size));

        m_pClippingPlaneNearEdit->setText(QString::number(Near));

        m_pClippingPlaneFarEdit->setText(QString::number(Far));

        m_pClearDepthEdit->setText(QString::number(Depth));
       
        m_pCameraModeCB->setCurrentIndex(CameraMode);

        m_pShutterSpeedEdit->setText(QString::number(ShutterSpeed));

        m_pApertureEdit->setText(QString::number(Aperture));

        m_pISOEdit->setText(QString::number(ISO));

        m_pECEdit->setText(QString::number(EC));

        m_pViewportXEdit->setText(QString::number(X));

        m_pViewportYEdit->setText(QString::number(Y));

        m_pViewportWEdit->setText(QString::number(W));

        m_pViewportHEdit->setText(QString::number(H));

        m_pFieldOfViewSlider->blockSignals(false);
        m_pFieldOfViewEdit  ->blockSignals(false);
        m_pClearFlagCS      ->blockSignals(false);
        m_pProjectionCB     ->blockSignals(false);
        m_pCameraModeCB     ->blockSignals(false);
    }
} // namespace Edit
