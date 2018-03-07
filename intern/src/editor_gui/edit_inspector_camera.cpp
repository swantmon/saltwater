
#include "base/base_include_glm.h"

#include "editor_gui/edit_inspector_camera.h"

#include "editor_port/edit_message_manager.h"

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
        int ClearFlag = m_pClearFlagCS->currentIndex();

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

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(ClearFlag);

        NewMessage.Put(AlbedoColor[0]);
        NewMessage.Put(AlbedoColor[1]);
        NewMessage.Put(AlbedoColor[2]);

        NewMessage.Put(CullingMask);

        NewMessage.Put(ProjectionType);

        NewMessage.Put(Size);

        NewMessage.Put(FOV);

        NewMessage.Put(Near);

        NewMessage.Put(Far);

        NewMessage.Put(X);
        NewMessage.Put(Y);
        NewMessage.Put(W);
        NewMessage.Put(H);

        NewMessage.Put(Depth);

        NewMessage.Put(CameraMode);

        NewMessage.Put(ShutterSpeed);

        NewMessage.Put(Aperture);

        NewMessage.Put(ISO);

        NewMessage.Put(EC);

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

    void CInspectorCamera::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

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
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        // -----------------------------------------------------------------------------
        // Get values
        // -----------------------------------------------------------------------------
        int ClearFlag = _rMessage.Get<int>();
    
        R = _rMessage.Get<float>();
        G = _rMessage.Get<float>();
        B = _rMessage.Get<float>();

        int CullingMask = _rMessage.Get<int>();

        BASE_UNUSED(CullingMask);

        int ProjectionType = _rMessage.Get<int>();

        float Size = _rMessage.Get<float>();

        float FOV = _rMessage.Get<float>();

        float Near = _rMessage.Get<float>();

        float Far = _rMessage.Get<float>();

        X = _rMessage.Get<float>();
        Y = _rMessage.Get<float>();
        W = _rMessage.Get<float>();
        H = _rMessage.Get<float>();

        float Depth = _rMessage.Get<float>();

        int CameraMode = _rMessage.Get<int>();

        float ShutterSpeed = _rMessage.Get<float>();

        float Aperture = _rMessage.Get<float>();

        float ISO = _rMessage.Get<float>();

        float EC = _rMessage.Get<float>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pFieldOfViewSlider->blockSignals(true);
        m_pFieldOfViewEdit  ->blockSignals(true);
        m_pClearFlagCS      ->blockSignals(true);
        m_pProjectionCB     ->blockSignals(true);
        m_pCameraModeCB     ->blockSignals(true);

        m_pClearFlagCS->setCurrentIndex(ClearFlag);

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
