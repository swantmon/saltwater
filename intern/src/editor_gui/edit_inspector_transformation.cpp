
#include "base/base_console.h"

#include "editor_gui/edit_inspector_transformation.h"

namespace Edit
{
    CInspectorTransformation::CInspectorTransformation(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(static_cast<unsigned int>(-1))
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::Entity_Transformation_Info, EDIT_RECEIVE_MESSAGE(&CInspectorTransformation::OnEntityInfoTransformation));
    }

    // -----------------------------------------------------------------------------

    CInspectorTransformation::~CInspectorTransformation()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::valueChanged()
    {
        float TranslationX;
        float TranslationY;
        float TranslationZ;
        float RotationX;
        float RotationY;
        float RotationZ;
        float ScaleX;
        float ScaleY;
        float ScaleZ;

        // -----------------------------------------------------------------------------
        // Read value
        // -----------------------------------------------------------------------------
        TranslationX = m_pTransformationPositionX->text().toFloat();
        TranslationY = m_pTransformationPositionY->text().toFloat();
        TranslationZ = m_pTransformationPositionZ->text().toFloat();

        RotationX = m_pTransformationRotationX->text().toFloat();
        RotationY = m_pTransformationRotationY->text().toFloat();
        RotationZ = m_pTransformationRotationZ->text().toFloat();

        ScaleX = m_pTransformationScaleX->text().toFloat();
        ScaleY = m_pTransformationScaleY->text().toFloat();
        ScaleZ = m_pTransformationScaleZ->text().toFloat();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Put(TranslationX);
        NewMessage.Put(TranslationY);
        NewMessage.Put(TranslationZ);

        NewMessage.Put(RotationX);
        NewMessage.Put(RotationY);
        NewMessage.Put(RotationZ);

        NewMessage.Put(ScaleX);
        NewMessage.Put(ScaleY);
        NewMessage.Put(ScaleZ);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Transformation_Update, NewMessage);
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

    void CInspectorTransformation::RequestInformation(Base::ID _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.Put(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::Entity_Transformation_Info, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorTransformation::OnEntityInfoTransformation(Edit::CMessage& _rMessage)
    {
        float TranslationX;
        float TranslationY;
        float TranslationZ;
        float RotationX;
        float RotationY;
        float RotationZ;
        float ScaleX;
        float ScaleY;
        float ScaleZ;

        bool  HasTransformation;
        
        Base::ID EntityID = _rMessage.Get<Base::ID>();

        if (EntityID != m_CurrentEntityID)
        {
            return;
        }

        HasTransformation = _rMessage.Get<bool>();

        if (HasTransformation)
        {
            TranslationX = _rMessage.Get<float>();
            TranslationY = _rMessage.Get<float>();
            TranslationZ = _rMessage.Get<float>();

            RotationX = _rMessage.Get<float>();
            RotationY = _rMessage.Get<float>();
            RotationZ = _rMessage.Get<float>();

            ScaleX = _rMessage.Get<float>();
            ScaleY = _rMessage.Get<float>();
            ScaleZ = _rMessage.Get<float>();

            m_pTransformationRotationX->setEnabled(true);
            m_pTransformationRotationY->setEnabled(true);
            m_pTransformationRotationZ->setEnabled(true);

            m_pTransformationScaleX->setEnabled(true);
            m_pTransformationScaleY->setEnabled(true);
            m_pTransformationScaleZ->setEnabled(true);
        }
        else
        {
            TranslationX = _rMessage.Get<float>();
            TranslationY = _rMessage.Get<float>();
            TranslationZ = _rMessage.Get<float>();

            RotationX = 0.0f;
            RotationY = 0.0f;
            RotationZ = 0.0f;

            ScaleX = 0.0f;
            ScaleY = 0.0f;
            ScaleZ = 0.0f;

            m_pTransformationRotationX->setEnabled(false);
            m_pTransformationRotationY->setEnabled(false);
            m_pTransformationRotationZ->setEnabled(false);

            m_pTransformationScaleX->setEnabled(false);
            m_pTransformationScaleY->setEnabled(false);
            m_pTransformationScaleZ->setEnabled(false);
        }

        m_pTransformationPositionX->setText(QString::number(TranslationX));
        m_pTransformationPositionY->setText(QString::number(TranslationY));
        m_pTransformationPositionZ->setText(QString::number(TranslationZ));

        m_pTransformationRotationX->setText(QString::number(RotationX));
        m_pTransformationRotationY->setText(QString::number(RotationY));
        m_pTransformationRotationZ->setText(QString::number(RotationZ));

        m_pTransformationScaleX->setText(QString::number(ScaleX));
        m_pTransformationScaleY->setText(QString::number(ScaleY));
        m_pTransformationScaleZ->setText(QString::number(ScaleZ));
    }
} // namespace Edit
