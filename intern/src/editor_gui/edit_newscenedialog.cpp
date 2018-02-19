#include "edit_newscenedialog.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

namespace Edit
{
    CNewSceneDialog::CNewSceneDialog(QObject* _pParent) 
        : QDialog() 
    {
        BASE_UNUSED(_pParent);

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

        int SizeX = m_pEditX->text().toInt();
        int SizeY = m_pEditY->text().toInt();

        NewMessage.Put(SizeX);
        NewMessage.Put(SizeY);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::App_NewMap, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CNewSceneDialog::pressCancel()
    {
    }
} // namespace Edit