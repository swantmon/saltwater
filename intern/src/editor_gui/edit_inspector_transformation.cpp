
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

        NewMessage.PutFloat((float)m_pTransformationPositionX->value());
        NewMessage.PutFloat((float)m_pTransformationPositionY->value());
        NewMessage.PutFloat((float)m_pTransformationPositionZ->value());

        NewMessage.PutFloat((float)m_pTransformationScaleX->value());
        NewMessage.PutFloat((float)m_pTransformationScaleY->value());
        NewMessage.PutFloat((float)m_pTransformationScaleZ->value());

        NewMessage.PutFloat((float)m_pTransformationRotationX->value());
        NewMessage.PutFloat((float)m_pTransformationRotationY->value());
        NewMessage.PutFloat((float)m_pTransformationRotationZ->value());

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::EntityInfoTransformation, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::OnEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        bool HasTransformation = _rMessage.GetBool();

        if (HasTransformation)
        {
            m_pTransformationPositionX->setValue(_rMessage.GetFloat());
            m_pTransformationPositionY->setValue(_rMessage.GetFloat());
            m_pTransformationPositionZ->setValue(_rMessage.GetFloat());

            m_pTransformationScaleX->setValue(_rMessage.GetFloat());
            m_pTransformationScaleY->setValue(_rMessage.GetFloat());
            m_pTransformationScaleZ->setValue(_rMessage.GetFloat());

            m_pTransformationRotationX->setValue(_rMessage.GetFloat());
            m_pTransformationRotationY->setValue(_rMessage.GetFloat());
            m_pTransformationRotationZ->setValue(_rMessage.GetFloat());
        }
        else
        {
            m_pTransformationPositionX->setValue(_rMessage.GetFloat());
            m_pTransformationPositionY->setValue(_rMessage.GetFloat());
            m_pTransformationPositionZ->setValue(_rMessage.GetFloat());

            m_pTransformationScaleX->setValue(0.0);
            m_pTransformationScaleY->setValue(0.0);
            m_pTransformationScaleZ->setValue(0.0);

            m_pTransformationRotationX->setValue(0.0);
            m_pTransformationRotationY->setValue(0.0);
            m_pTransformationRotationZ->setValue(0.0);
        }
    }
} // namespace Edit
