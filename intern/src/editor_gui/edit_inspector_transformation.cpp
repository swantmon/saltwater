
#include "base/base_console.h"

#include "editor_gui/edit_inspector_transformation.h"

namespace Edit
{
    CInspectorTransformation::CInspectorTransformation(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::EntityInfoTransformation, EDIT_RECEIVE_MESSAGE(&CInspectorTransformation::OnEntityInfoTransformation));
    }

    // -----------------------------------------------------------------------------

    CInspectorTransformation::~CInspectorTransformation()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::valueChanged()
    {
        CMessage NewMessage;

        NewMessage.PutFloat(m_pTransformationPositionZ->text().toFloat());
        NewMessage.PutFloat(m_pTransformationPositionY->text().toFloat());
        NewMessage.PutFloat(m_pTransformationPositionX->text().toFloat());

        NewMessage.PutFloat(m_pTransformationRotationZ->text().toFloat());
        NewMessage.PutFloat(m_pTransformationRotationY->text().toFloat());
        NewMessage.PutFloat(m_pTransformationRotationX->text().toFloat());

        NewMessage.PutFloat(m_pTransformationScaleZ->text().toFloat());
        NewMessage.PutFloat(m_pTransformationScaleY->text().toFloat());
        NewMessage.PutFloat(m_pTransformationScaleX->text().toFloat());

        BASE_CONSOLE_INFO("Send values:");
        BASE_CONSOLE_INFOV("Pos: %f, %f, %f", m_pTransformationPositionX->text().toFloat(), m_pTransformationPositionY->text().toFloat(), m_pTransformationPositionZ->text().toFloat());
        BASE_CONSOLE_INFOV("Rot: %f, %f, %f", m_pTransformationRotationX->text().toFloat(), m_pTransformationRotationY->text().toFloat(), m_pTransformationRotationZ->text().toFloat());
        BASE_CONSOLE_INFOV("Sca: %f, %f, %f", m_pTransformationScaleX->text().toFloat(), m_pTransformationScaleY->text().toFloat(), m_pTransformationScaleZ->text().toFloat());
        BASE_CONSOLE_INFO("--------------------------------");

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::EntityInfoTransformation, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::OnEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        bool HasTransformation = _rMessage.GetBool();

        if (HasTransformation)
        {
            m_pTransformationPositionX->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationPositionY->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationPositionZ->setText(QString::number(_rMessage.GetFloat()));

            m_pTransformationRotationX->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationRotationY->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationRotationZ->setText(QString::number(_rMessage.GetFloat()));

            m_pTransformationScaleX->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationScaleY->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationScaleZ->setText(QString::number(_rMessage.GetFloat()));

            m_pTransformationRotationX->setEnabled(true);
            m_pTransformationRotationY->setEnabled(true);
            m_pTransformationRotationZ->setEnabled(true);

            m_pTransformationScaleX->setEnabled(true);
            m_pTransformationScaleY->setEnabled(true);
            m_pTransformationScaleZ->setEnabled(true);
        }
        else
        {
            m_pTransformationPositionX->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationPositionY->setText(QString::number(_rMessage.GetFloat()));
            m_pTransformationPositionZ->setText(QString::number(_rMessage.GetFloat()));

            m_pTransformationRotationX->setText("0");
            m_pTransformationRotationY->setText("0");
            m_pTransformationRotationZ->setText("0");

            m_pTransformationScaleX->setText("0");
            m_pTransformationScaleY->setText("0");
            m_pTransformationScaleZ->setText("0");            

            m_pTransformationRotationX->setEnabled(false);
            m_pTransformationRotationY->setEnabled(false);
            m_pTransformationRotationZ->setEnabled(false);

            m_pTransformationScaleX->setEnabled(false);
            m_pTransformationScaleY->setEnabled(false);
            m_pTransformationScaleZ->setEnabled(false);
        }
    }
} // namespace Edit
