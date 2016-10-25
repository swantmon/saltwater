#include "edit_newscenedialog.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CNewSceneDialog::CNewSceneDialog(QObject* _pParent) 
        : QDialog() 
    {
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CNewSceneDialog::~CNewSceneDialog() 
    {
	
    }

    // -----------------------------------------------------------------------------

    void CNewSceneDialog::pressOkay()
    {
        // -----------------------------------------------------------------------------
        // Send message with new scene / map request
        // -----------------------------------------------------------------------------
        CMessage NewMessage;

        int SizeX = m_pEditX->toPlainText().toInt();
        int SizeY = m_pEditY->toPlainText().toInt();

        NewMessage.PutInt(SizeX);
        NewMessage.PutInt(SizeY);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewMap, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CNewSceneDialog::pressCancel()
    {
    }
} // namespace Edit