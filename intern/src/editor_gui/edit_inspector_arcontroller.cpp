
#include "editor_gui/edit_inspector_arcontroller.h"
#include "editor_gui/edit_inspector_arcontroller_marker.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorARController::CInspectorARController(QWidget* _pParent)
        : QWidget(_pParent)
        , m_CurrentEntityID(-1)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::PluginInfoARController, EDIT_RECEIVE_MESSAGE(&CInspectorARController::OnEntityInfoARController));
    }

    // -----------------------------------------------------------------------------

    CInspectorARController::~CInspectorARController()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int Device = m_pDeviceCB->currentIndex();

        QString ParameterFile = m_pParameterFile->text();
        QByteArray ParameterFileBinary = ParameterFile.toLatin1();

        unsigned int DeviceNumber = m_pParameterFile->text().toUInt();

        unsigned int CameraEntityID = m_pCameraEntityIDEdit->text().toUInt();

        unsigned int NumberOfMarker = m_pNumberOfMarkerEdit->text().toUInt();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(Device);

        NewMessage.PutString(ParameterFileBinary.data());

        NewMessage.PutInt(DeviceNumber);

        NewMessage.PutInt(CameraEntityID);

        NewMessage.PutInt(NumberOfMarker);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::PluginInfoARController, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::numberOfMarkerChanged()
    {
        unsigned int CurrentNumberOfMarker = m_pMarkerLayout->count();
        unsigned int NewNumberOfMarker     = m_pNumberOfMarkerEdit->text().toUInt();

        if (CurrentNumberOfMarker == NewNumberOfMarker)
        {
            return;
        }

        unsigned int CurrentAvailableMarkerIndex = 0;

        for (CurrentAvailableMarkerIndex = 0; CurrentAvailableMarkerIndex < m_pMarkerLayout->count(); ++CurrentAvailableMarkerIndex)
        {
            CInspectorARControllerMarker* pWidget = static_cast<CInspectorARControllerMarker*>(m_pMarkerLayout->itemAt(CurrentAvailableMarkerIndex)->widget());

            if (CurrentAvailableMarkerIndex < NewNumberOfMarker)
            {
                pWidget->setVisible(true);

                pWidget->RequestInformation(m_CurrentEntityID, CurrentAvailableMarkerIndex);
            }
            else
            {
                pWidget->setVisible(false);
            }
        }

        for (unsigned int CurrentMarkerIndex = CurrentAvailableMarkerIndex; CurrentMarkerIndex < NewNumberOfMarker; ++CurrentMarkerIndex)
        {
            CInspectorARControllerMarker* pWidget = new CInspectorARControllerMarker();

            m_pMarkerLayout->addWidget(pWidget);

            pWidget->RequestInformation(m_CurrentEntityID, CurrentMarkerIndex);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestPluginInfoARController, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::OnEntityInfoARController(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID) return;

        int Device = _rMessage.GetInt();

        char Text[256];

        _rMessage.GetString(Text, 256);

        unsigned int DeviceNumber = _rMessage.GetInt();

        int CameraEntityID = _rMessage.GetInt();

        unsigned int OutputBackground = _rMessage.GetInt();

        unsigned int OutputCubemap = _rMessage.GetInt();

        unsigned int NumberOfMarker = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pDeviceCB->blockSignals(true);

        m_pDeviceCB->setCurrentIndex(Device);

        m_pParameterFile->setText(Text);

        m_pDeviceNumberEdit->setText(QString::number(DeviceNumber));

        if (CameraEntityID >= 0)
        {
            m_pCameraEntityIDEdit->setText(QString::number(CameraEntityID));
        }

        m_pOutputBackgroundEdit->setText(QString::number(OutputBackground));

        m_pOutputCubemapEdit->setText(QString::number(OutputCubemap));

        m_pNumberOfMarkerEdit->setText(QString::number(NumberOfMarker));

        m_pDeviceCB->blockSignals(false);
    }
} // namespace Edit
