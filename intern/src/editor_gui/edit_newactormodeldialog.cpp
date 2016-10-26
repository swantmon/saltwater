
#include "editor_gui/edit_newactormodeldialog.h"

#include "editor_port/edit_message.h"
#include "editor_port/edit_message_manager.h"

#include <QFileDialog>

namespace Edit
{
    CNewActorModelDialog::CNewActorModelDialog(QObject* _pParent) 
        : QDialog       ()
        , m_ModelFile   ()
        , m_MaterialFile()
    {
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CNewActorModelDialog::~CNewActorModelDialog() 
    {

    }

    // -----------------------------------------------------------------------------

    void CNewActorModelDialog::openFileDialogModel()
    {
        m_ModelFile = QFileDialog::getOpenFileName(this, tr("Load model file"), tr(""), tr("Object files (*.obj *.dae)"));

        m_pEditModel->setText(m_ModelFile);
    }

    // -----------------------------------------------------------------------------

    void CNewActorModelDialog::openFileDialogMaterial()
    {
        m_MaterialFile = QFileDialog::getOpenFileName(this, tr("Load material file"), tr(""), tr("Material files (*.mat)"));

        m_pEditMaterial->setText(m_MaterialFile);
    }

    // -----------------------------------------------------------------------------

    void CNewActorModelDialog::pressOkay()
    {
        // -----------------------------------------------------------------------------
        // Send message with new scene / map request
        // -----------------------------------------------------------------------------
        std::string SaveCopy;
        CMessage NewMessage;

        if (!m_ModelFile.isEmpty())
        {
            SaveCopy = m_ModelFile.toStdString();

            NewMessage.PutBool(true);
            NewMessage.PutString(SaveCopy.c_str());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        if (!m_MaterialFile.isEmpty())
        {
            SaveCopy = m_MaterialFile.toStdString();

            NewMessage.PutBool(true);
            NewMessage.PutString(SaveCopy.c_str());
        }
        else
        {
            NewMessage.PutBool(false);
        }

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::NewEntityActor, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CNewActorModelDialog::pressCancel()
    {

    }
} // namespace Edit
