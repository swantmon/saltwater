
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
        Edit::MessageManager::Register(Edit::SApplicationMessageType::PluginInfoARControllerMarker, EDIT_RECEIVE_MESSAGE(&CInspectorARControllerMarker::OnEntityInfoARControllerMarker));
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

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(m_CurrentMarkerID);

        NewMessage.PutInt(UID);

        NewMessage.PutInt(Type);

        NewMessage.PutString(PatternFileBinary.data());

        NewMessage.PutFloat(Width);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::PluginInfoARControllerMarker, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARControllerMarker::RequestInformation(unsigned int _EntityID, unsigned int _MarkerID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestPluginInfoARControllerMarker, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorARControllerMarker::OnEntityInfoARControllerMarker(Edit::CMessage& _rMessage)
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();

        int MarkerID = _rMessage.GetInt();

        if (EntityID != m_CurrentEntityID || MarkerID != m_CurrentMarkerID) return;

        int UID = _rMessage.GetInt();

        int Type = _rMessage.GetInt();

        char Text[256];

        _rMessage.GetString(Text, 256);

        float Width = _rMessage.GetFloat();

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
