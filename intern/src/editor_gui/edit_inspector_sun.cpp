
#include "base/base_vector3.h"

#include "editor_gui/edit_inspector_sun.h"

#include "editor_port/edit_message_manager.h"

#include <QColorDialog>

namespace Edit
{
    CInspectorSun::CInspectorSun(QWidget* _pParent)
        : QWidget          (_pParent)
        , m_CurrentEntityID(-1)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);

        // -----------------------------------------------------------------------------
        // Color picker
        // -----------------------------------------------------------------------------
        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Qt::white));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        // -----------------------------------------------------------------------------
        // Messages
        // -----------------------------------------------------------------------------
        Edit::MessageManager::Register(Edit::SApplicationMessageType::LightInfoSun, EDIT_RECEIVE_MESSAGE(&CInspectorSun::OnEntityInfoSun));
    }

    // -----------------------------------------------------------------------------

    CInspectorSun::~CInspectorSun()
    {

    }

    // -----------------------------------------------------------------------------

    void CInspectorSun::valueChanged()
    {
        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int ColorMode = m_pColorModeCB->currentIndex();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        QColor RGB = ButtonPalette.color(QPalette::Button);

        Base::Float3 Color = Base::Float3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        float Temperature = m_pTemperatureEdit->text().toFloat();
        float Intensity   = m_pIntensityEdit->text().toFloat();

        Base::Float3 Direction = Base::Float3(m_pDirectionXEdit->text().toFloat(), m_pDirectionYEdit->text().toFloat(), m_pDirectionZEdit->text().toFloat());

        int ShadowRefresh = m_pShadowRefreshCB->currentIndex();

        // -----------------------------------------------------------------------------
        // Send message
        // -----------------------------------------------------------------------------
        Edit::CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.PutInt(ColorMode);

        NewMessage.PutFloat(Color[0]);
        NewMessage.PutFloat(Color[1]);
        NewMessage.PutFloat(Color[2]);

        NewMessage.PutFloat(Temperature);
        NewMessage.PutFloat(Intensity);

        NewMessage.PutFloat(Direction[0]);
        NewMessage.PutFloat(Direction[1]);
        NewMessage.PutFloat(Direction[2]);

        NewMessage.PutInt(ShadowRefresh);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::LightInfoSun, NewMessage);

    }

    // -----------------------------------------------------------------------------

    void CInspectorSun::pickColorFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        valueChanged();
    }

    // -----------------------------------------------------------------------------

    void CInspectorSun::RequestInformation(unsigned int _EntityID)
    {
        m_CurrentEntityID = _EntityID;

        CMessage NewMessage;

        NewMessage.PutInt(m_CurrentEntityID);

        NewMessage.Reset();

        MessageManager::SendMessage(SGUIMessageType::RequestLightInfoSun, NewMessage);
    }

    // -----------------------------------------------------------------------------

    void CInspectorSun::OnEntityInfoSun(Edit::CMessage& _rMessage)
    {
        float X, Y, Z;

        // -----------------------------------------------------------------------------
        // Read values
        // -----------------------------------------------------------------------------
        int EntityID = _rMessage.GetInt();
        int ColorMode = _rMessage.GetInt();

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();

        Base::Int3 Color = Base::Int3(X * 255, Y * 255, Z * 255);

        float Temperature = _rMessage.GetFloat();
        float Intensity   = _rMessage.GetFloat();

        X = _rMessage.GetFloat();
        Y = _rMessage.GetFloat();
        Z = _rMessage.GetFloat();

        Base::Float3 Direction = Base::Float3(X, Y, Z);

        int ShadowRefresh = _rMessage.GetInt();

        // -----------------------------------------------------------------------------
        // Set values
        // -----------------------------------------------------------------------------
        m_pColorModeCB    ->blockSignals(true);
        m_pShadowRefreshCB->blockSignals(true);

        m_pColorModeCB->setCurrentIndex(ColorMode);

        QPalette ButtonPalette = m_pPickColorButton->palette();

        ButtonPalette.setColor(QPalette::Button, QColor(Color[0], Color[1], Color[2]));

        m_pPickColorButton->setPalette(ButtonPalette);

        m_pPickColorButton->update();

        m_pTemperatureEdit->setText(QString::number(Temperature));
        m_pIntensityEdit  ->setText(QString::number(Intensity));

        m_pDirectionXEdit->setText(QString::number(Direction[0]));
        m_pDirectionYEdit->setText(QString::number(Direction[1]));
        m_pDirectionZEdit->setText(QString::number(Direction[2]));

        m_pShadowRefreshCB->setCurrentIndex(ShadowRefresh);

        m_pColorModeCB    ->blockSignals(false);
        m_pShadowRefreshCB->blockSignals(false);
    }
} // namespace Edit
