
#include "editor_gui/edit_inspector_arcontroller.h"
#include "editor_gui/edit_inspector_arcontroller_marker.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorARController::CInspectorARController(QWidget* _pParent)
        : QWidget(_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Plugin_ARController_Info, EDIT_RECEIVE_MESSAGE(&CInspectorARController::OnEntityInfoARController));
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

        bool FreezeLastFrame = m_pFreezeLastFrameCB->isChecked();

        QString Configuration = m_pConfigurationEdit->text();
        QByteArray ConfigurationFileBinary = Configuration.toLatin1();

        QString ParameterFile = m_pParameterFile->text();
        QByteArray ParameterFileBinary = ParameterFile.toLatin1();

        Base::ID CameraEntityID = m_pCameraEntityIDEdit->text().toUInt();

        unsigned int NumberOfMarker = m_pNumberOfMarkerEdit->text().toUInt();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(Device);

        NewMessage.Put(FreezeLastFrame);

        NewMessage.Put(std::string(ConfigurationFileBinary.data()));

        NewMessage.Put(std::string(ParameterFileBinary.data()));

        NewMessage.Put(CameraEntityID);

        NewMessage.Put(NumberOfMarker);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Plugin_ARConroller_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::numberOfMarkerChanged()
    {
        int CurrentNumberOfMarker = m_pMarkerLayout->count();
        int NewNumberOfMarker     = m_pNumberOfMarkerEdit->text().toUInt();

        if (CurrentNumberOfMarker == NewNumberOfMarker)
        {
            return;
        }

        int CurrentAvailableMarkerIndex = 0;

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

        for (int CurrentMarkerIndex = CurrentAvailableMarkerIndex; CurrentMarkerIndex < NewNumberOfMarker; ++CurrentMarkerIndex)
        {
            CInspectorARControllerMarker* pWidget = new CInspectorARControllerMarker();

            m_pMarkerLayout->addWidget(pWidget);

            pWidget->RequestInformation(m_CurrentEntityID, CurrentMarkerIndex);
        }
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Plugin_ARConroller_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARController::OnEntityInfoARController(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID) return;

        int Device = _rMessage.Get<int>();

        bool FreezeLastFrame = _rMessage.Get<bool>();

        std::string Configuration = _rMessage.Get<std::string>();

        std::string ParameterFile = _rMessage.Get<std::string>();

        Base::ID CameraEntityID = _rMessage.Get<Base::ID>();

        unsigned int OutputBackground = _rMessage.Get<int>();

        unsigned int NumberOfMarker = _rMessage.Get<int>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pDeviceCB->blockSignals(true);
        m_pFreezeLastFrameCB->blockSignals(true);

        m_pDeviceCB->setCurrentIndex(Device);

        m_pFreezeLastFrameCB->setChecked(FreezeLastFrame);

        m_pConfigurationEdit->setText(QString(Configuration.c_str()));

        m_pParameterFile->setText(QString(ParameterFile.c_str()));

        if (CameraEntityID >= 0)
        {
            m_pCameraEntityIDEdit->setText(QString::number(CameraEntityID));
        }

        m_pOutputBackgroundEdit->setText(QString::number(OutputBackground));

        m_pNumberOfMarkerEdit->setText(QString::number(NumberOfMarker));

        m_pFreezeLastFrameCB->blockSignals(false);
        m_pDeviceCB->blockSignals(false);
    }
} // namespace Edit
