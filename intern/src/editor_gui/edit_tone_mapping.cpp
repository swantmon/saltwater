
#include "edit_tone_mapping.h"

#include "base/base_include_glm.h"

#include <QColorDialog>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

namespace Edit
{
    CToneMapping::CToneMapping(QWidget* _pParent)
        : QWidget(_pParent)
    {
        // -----------------------------------------------------------------------------
        // Setup UI
        // -----------------------------------------------------------------------------
        setupUi(this);
    }

    // -----------------------------------------------------------------------------

    CToneMapping::~CToneMapping() 
    {
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::pickTintFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickTintButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickTintButton->setPalette(ButtonPalette);

        m_pPickTintButton->update();

        changeToneMappingSettings();
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::pickDarkTintFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickDarkTintButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickDarkTintButton->setPalette(ButtonPalette);

        m_pPickDarkTintButton->update();

        changeToneMappingSettings();
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::pickRedFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickRedButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickRedButton->setPalette(ButtonPalette);

        m_pPickRedButton->update();

        changeToneMappingSettings();
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::pickGreenFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickGreenButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickGreenButton->setPalette(ButtonPalette);

        m_pPickGreenButton->update();

        changeToneMappingSettings();
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::pickBlueFromDialog()
    {
        QColor NewColor = QColorDialog::getColor();

        QPalette ButtonPalette = m_pPickBlueButton->palette();

        ButtonPalette.setColor(QPalette::Button, NewColor);

        m_pPickBlueButton->setPalette(ButtonPalette);

        m_pPickBlueButton->update();

        changeToneMappingSettings();
    }

    // -----------------------------------------------------------------------------

    void CToneMapping::changeToneMappingSettings()
    {
        glm::vec3 Tint;
        glm::vec3 DarkTint;
        float DarkTintBlend;
        float DarkTintStrength;
        float Saturation;
        float Contrast;
        float DynamicRange;
        float CrushBrights;
        float CrushDarks;
        glm::vec3 Red;
        glm::vec3 Green;
        glm::vec3 Blue;
        QColor RGB;
        glm::vec3 Color;
        Edit::CMessage NewMessage;

        // -----------------------------------------------------------------------------

        RGB = m_pPickTintButton->palette().color(QPalette::Button);

        Tint = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        RGB = m_pPickDarkTintButton->palette().color(QPalette::Button);

        DarkTint = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        RGB = m_pPickRedButton->palette().color(QPalette::Button);

        Red = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        RGB = m_pPickGreenButton->palette().color(QPalette::Button);

        Green = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        RGB = m_pPickBlueButton->palette().color(QPalette::Button);

        Blue = glm::vec3(RGB.red() / 255.0f, RGB.green() / 255.0f, RGB.blue() / 255.0f);

        DarkTintBlend = m_pDarkTintBlendEdit->text().toFloat();

        DarkTintStrength = m_pDarkTintStrengthEdit->text().toFloat();

        Saturation = m_pSaturationEdit->text().toFloat();

        Contrast = m_pContrastEdit->text().toFloat();

        DynamicRange = m_pDynamicRangeEdit->text().toFloat();

        CrushBrights = m_pCrushBrightsEdit->text().toFloat();

        CrushDarks = m_pCrushDarksEdit->text().toFloat();

        NewMessage.PutFloat(Tint[0]);
        NewMessage.PutFloat(Tint[1]);
        NewMessage.PutFloat(Tint[2]);

        NewMessage.PutFloat(DarkTint[0]);
        NewMessage.PutFloat(DarkTint[1]);
        NewMessage.PutFloat(DarkTint[2]);

        NewMessage.PutFloat(DarkTintBlend);

        NewMessage.PutFloat(DarkTintStrength);

        NewMessage.PutFloat(Saturation);

        NewMessage.PutFloat(Contrast);

        NewMessage.PutFloat(DynamicRange);

        NewMessage.PutFloat(CrushBrights);

        NewMessage.PutFloat(CrushDarks);

        NewMessage.PutFloat(Red[0]);
        NewMessage.PutFloat(Red[1]);
        NewMessage.PutFloat(Red[2]);

        NewMessage.PutFloat(Green[0]);
        NewMessage.PutFloat(Green[1]);
        NewMessage.PutFloat(Green[2]);

        NewMessage.PutFloat(Blue[0]);
        NewMessage.PutFloat(Blue[1]);
        NewMessage.PutFloat(Blue[2]);

        NewMessage.Reset();

        Edit::MessageManager::SendMessage(Edit::SGUIMessageType::Graphic_ToneMapping_Update, NewMessage);
    }
} // namespace Edit