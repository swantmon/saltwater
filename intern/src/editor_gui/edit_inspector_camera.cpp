
#include "base/base_vector3.h"

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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::ActorInfoCamera, EDIT_RECEIVE_MESSAGE(&CInspectorCamera::OnEntityInfoCamera));
    }

    // -----------------------------------------------------------------------------

    CInspectorCamera::~CInspectorCamera() 
    {

    }
    // -----------------------------------------------------------------------------

    void CInspectorCamera::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pSolidColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 AlbedoColor = Base::Float3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        float FOV = m_pFieldOfViewEdit->text().toFloat();

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


        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::ActorInfoCamera, NewMessage);
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

        MessageManager::SendMessage(SGUIMessageType::RequestActorInfoCamera, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorCamera::OnEntityInfoCamera(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;



        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
    }
} // namespace Edit
