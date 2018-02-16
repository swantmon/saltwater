
#include "editor_gui/edit_inspector_arcontroller_marker.h"

#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CInspectorARControllerMarker::CInspectorARControllerMarker(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID()
        , m_CurrentMarkerID()
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Plugin_ARController_Marker_Info, EDIT_RECEIVE_MESSAGE(&CInspectorARControllerMarker::OnInfoARControllerMarker));
    }

    // -----------------------------------------------------------------------------

    CInspectorARControllerMarker::~CInspectorARControllerMarker()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorARControllerMarker::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        unsigned int UID = m_pUIDEdit->text().toUInt();

        unsigned int Type = m_pTypeCB->currentIndex();

        QString PatternFile = m_pPatternFileEdit->text();
        QByteArray PatternFileBinary = PatternFile.toLatin1();

        float Width = m_pWidthEdit->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(m_CurrentMarkerID);

        NewMessage.Put(UID);

        NewMessage.Put(Type);

        NewMessage.PutString(PatternFileBinary.data());

        NewMessage.Put(Width);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Plugin_ARConroller_Marker_Update, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARControllerMarker::RequestInformation(Base::ID _EntityID, unsigned int _MarkerID)
    {
        BASE_UNUSED(_MarkerID);

        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Plugin_ARConroller_Marker_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARControllerMarker::OnInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        int MarkerID = _rMessage.Get<int>();

        if (EntityID != m_CurrentEntityID || MarkerID != m_CurrentMarkerID) return;

        int UID = _rMessage.Get<int>();

        int Type = _rMessage.Get<int>();

        char Text[256];

        _rMessage.GetString(Text, 256);

        float Width = _rMessage.Get<float>();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pTypeCB->blockSignals(true);

        m_pUIDEdit->setText(QString::number(UID));

        m_pTypeCB->setCurrentIndex(Type);

        m_pPatternFileEdit->setText(Text);

        m_pWidthEdit->setText(QString::number(Width));

        m_pTypeCB->blockSignals(false);
    }
} // namespace Edit
