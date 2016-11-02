
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

        // TODO:
        // Why is the Z before X???

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::EntityInfoTransformation, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changePositionX(QPoint _Difference)
    {
        float Difference = _Difference.x() / 100.0f;

        float PreviousValue = m_pTransformationPositionX->text().toFloat();

        m_pTransformationPositionX->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changePositionY(QPoint _Difference)
    {
        float Difference = _Difference.x() / 100.0f;

        float PreviousValue = m_pTransformationPositionY->text().toFloat();

        m_pTransformationPositionY->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changePositionZ(QPoint _Difference)
    {
        float Difference = _Difference.x() / 100.0f;

        float PreviousValue = m_pTransformationPositionZ->text().toFloat();

        m_pTransformationPositionZ->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeRotationX(QPoint _Difference)
    {
        float Difference = _Difference.x() / 10.0f;

        float PreviousValue = m_pTransformationRotationX->text().toFloat();

        m_pTransformationRotationX->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeRotationY(QPoint _Difference)
    {
        float Difference = _Difference.x() / 10.0f;

        float PreviousValue = m_pTransformationRotationY->text().toFloat();

        m_pTransformationRotationY->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeRotationZ(QPoint _Difference)
    {
        float Difference = _Difference.x() / 10.0f;

        float PreviousValue = m_pTransformationRotationZ->text().toFloat();

        m_pTransformationRotationZ->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeScaleX(QPoint _Difference)
    {
        float Difference = _Difference.x() / 1000.0f;

        float PreviousValue = m_pTransformationScaleX->text().toFloat();

        m_pTransformationScaleX->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeScaleY(QPoint _Difference)
    {
        float Difference = _Difference.x() / 1000.0f;

        float PreviousValue = m_pTransformationScaleY->text().toFloat();

        m_pTransformationScaleY->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::changeScaleZ(QPoint _Difference)
    {
        float Difference = _Difference.x() / 1000.0f;

        float PreviousValue = m_pTransformationScaleZ->text().toFloat();

        m_pTransformationScaleZ->setText(QString::number(PreviousValue + Difference));

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::RequestInformation()
    {
        CMessage NewMessage(true);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestEntityInfoTransformation, NewMessage);
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
