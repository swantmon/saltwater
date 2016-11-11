/********************************************************************************
** Form generated from reading UI file 'edit_inspector_camera.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDIT_INSPECTOR_CAMERA_H
#define UI_EDIT_INSPECTOR_CAMERA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InspectorCamera
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QLineEdit *m_pECEdit;
    QCheckBox *m_pOcclusionCullingCB;
    QLineEdit *m_pISOEdit;
    QLabel *label_16;
    QLabel *label_21;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *m_pSolidColorButton;
    QPushButton *m_pSolidColorPickButton;
    QComboBox *m_pClearFlagCS;
    QLineEdit *m_pClearDepthEdit;
    QComboBox *m_pProjectionCB;
    QLabel *label;
    QLabel *label_8;
    QLabel *label_9;
    QCheckBox *m_pIsMainCameraCB;
    QLabel *label_10;
    QLineEdit *m_pOrthographicSizeEdit;
    QComboBox *m_pCameraModeCB;
    QHBoxLayout *horizontalLayout_2;
    QSlider *m_pFieldOfViewSlider;
    QLineEdit *m_pFieldOfViewEdit;
    QLabel *label_4;
    QLabel *label_17;
    QLabel *label_19;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *m_pApertureEdit;
    QLabel *label_23;
    QLabel *label_20;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *m_pShutterSpeedEdit;
    QLabel *label_22;
    QLabel *label_18;
    QGridLayout *gridLayout_2;
    QLabel *label_6;
    QLabel *label_7;
    QLineEdit *m_pClippingPlaneNearEdit;
    QLineEdit *m_pClippingPlaneFarEdit;
    QLabel *label_24;
    QLineEdit *m_pBackgroundTextureEdit;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_11;
    QGridLayout *gridLayout_3;
    QLabel *label_14;
    QLabel *label_12;
    QLabel *label_13;
    QLabel *label_15;
    QLineEdit *m_pViewportXEdit;
    QLineEdit *m_pViewportWEdit;
    QLineEdit *m_pViewportYEdit;
    QLineEdit *m_pViewportHEdit;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *InspectorCamera)
    {
        if (InspectorCamera->objectName().isEmpty())
            InspectorCamera->setObjectName(QStringLiteral("InspectorCamera"));
        InspectorCamera->resize(448, 539);
        verticalLayout = new QVBoxLayout(InspectorCamera);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        groupBox = new QGroupBox(InspectorCamera);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        m_pECEdit = new QLineEdit(groupBox);
        m_pECEdit->setObjectName(QStringLiteral("m_pECEdit"));

        gridLayout->addWidget(m_pECEdit, 14, 1, 1, 1);

        m_pOcclusionCullingCB = new QCheckBox(groupBox);
        m_pOcclusionCullingCB->setObjectName(QStringLiteral("m_pOcclusionCullingCB"));
        m_pOcclusionCullingCB->setChecked(true);

        gridLayout->addWidget(m_pOcclusionCullingCB, 8, 1, 1, 1);

        m_pISOEdit = new QLineEdit(groupBox);
        m_pISOEdit->setObjectName(QStringLiteral("m_pISOEdit"));

        gridLayout->addWidget(m_pISOEdit, 13, 1, 1, 1);

        label_16 = new QLabel(groupBox);
        label_16->setObjectName(QStringLiteral("label_16"));

        gridLayout->addWidget(label_16, 9, 0, 1, 1);

        label_21 = new QLabel(groupBox);
        label_21->setObjectName(QStringLiteral("label_21"));

        gridLayout->addWidget(label_21, 14, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 4, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout->addWidget(label_5, 7, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        m_pSolidColorButton = new QPushButton(groupBox);
        m_pSolidColorButton->setObjectName(QStringLiteral("m_pSolidColorButton"));
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        m_pSolidColorButton->setPalette(palette);
        m_pSolidColorButton->setAutoFillBackground(true);
        m_pSolidColorButton->setFlat(true);

        horizontalLayout->addWidget(m_pSolidColorButton);

        m_pSolidColorPickButton = new QPushButton(groupBox);
        m_pSolidColorPickButton->setObjectName(QStringLiteral("m_pSolidColorPickButton"));
        m_pSolidColorPickButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(m_pSolidColorPickButton);


        gridLayout->addLayout(horizontalLayout, 2, 1, 1, 1);

        m_pClearFlagCS = new QComboBox(groupBox);
        m_pClearFlagCS->setObjectName(QStringLiteral("m_pClearFlagCS"));

        gridLayout->addWidget(m_pClearFlagCS, 1, 1, 1, 1);

        m_pClearDepthEdit = new QLineEdit(groupBox);
        m_pClearDepthEdit->setObjectName(QStringLiteral("m_pClearDepthEdit"));

        gridLayout->addWidget(m_pClearDepthEdit, 9, 1, 1, 1);

        m_pProjectionCB = new QComboBox(groupBox);
        m_pProjectionCB->setObjectName(QStringLiteral("m_pProjectionCB"));

        gridLayout->addWidget(m_pProjectionCB, 4, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 8, 0, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        m_pIsMainCameraCB = new QCheckBox(groupBox);
        m_pIsMainCameraCB->setObjectName(QStringLiteral("m_pIsMainCameraCB"));

        gridLayout->addWidget(m_pIsMainCameraCB, 0, 1, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout->addWidget(label_10, 6, 0, 1, 1);

        m_pOrthographicSizeEdit = new QLineEdit(groupBox);
        m_pOrthographicSizeEdit->setObjectName(QStringLiteral("m_pOrthographicSizeEdit"));

        gridLayout->addWidget(m_pOrthographicSizeEdit, 6, 1, 1, 1);

        m_pCameraModeCB = new QComboBox(groupBox);
        m_pCameraModeCB->setObjectName(QStringLiteral("m_pCameraModeCB"));

        gridLayout->addWidget(m_pCameraModeCB, 10, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        m_pFieldOfViewSlider = new QSlider(groupBox);
        m_pFieldOfViewSlider->setObjectName(QStringLiteral("m_pFieldOfViewSlider"));
        m_pFieldOfViewSlider->setMinimum(1);
        m_pFieldOfViewSlider->setMaximum(179);
        m_pFieldOfViewSlider->setValue(60);
        m_pFieldOfViewSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(m_pFieldOfViewSlider);

        m_pFieldOfViewEdit = new QLineEdit(groupBox);
        m_pFieldOfViewEdit->setObjectName(QStringLiteral("m_pFieldOfViewEdit"));
        m_pFieldOfViewEdit->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_2->addWidget(m_pFieldOfViewEdit);


        gridLayout->addLayout(horizontalLayout_2, 5, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 5, 0, 1, 1);

        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QStringLiteral("label_17"));

        gridLayout->addWidget(label_17, 10, 0, 1, 1);

        label_19 = new QLabel(groupBox);
        label_19->setObjectName(QStringLiteral("label_19"));

        gridLayout->addWidget(label_19, 12, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        m_pApertureEdit = new QLineEdit(groupBox);
        m_pApertureEdit->setObjectName(QStringLiteral("m_pApertureEdit"));

        horizontalLayout_4->addWidget(m_pApertureEdit);

        label_23 = new QLabel(groupBox);
        label_23->setObjectName(QStringLiteral("label_23"));

        horizontalLayout_4->addWidget(label_23);


        gridLayout->addLayout(horizontalLayout_4, 12, 1, 1, 1);

        label_20 = new QLabel(groupBox);
        label_20->setObjectName(QStringLiteral("label_20"));

        gridLayout->addWidget(label_20, 13, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        m_pShutterSpeedEdit = new QLineEdit(groupBox);
        m_pShutterSpeedEdit->setObjectName(QStringLiteral("m_pShutterSpeedEdit"));

        horizontalLayout_3->addWidget(m_pShutterSpeedEdit);

        label_22 = new QLabel(groupBox);
        label_22->setObjectName(QStringLiteral("label_22"));

        horizontalLayout_3->addWidget(label_22);


        gridLayout->addLayout(horizontalLayout_3, 11, 1, 1, 1);

        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QStringLiteral("label_18"));

        gridLayout->addWidget(label_18, 11, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_2->addWidget(label_6, 0, 0, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout_2->addWidget(label_7, 1, 0, 1, 1);

        m_pClippingPlaneNearEdit = new QLineEdit(groupBox);
        m_pClippingPlaneNearEdit->setObjectName(QStringLiteral("m_pClippingPlaneNearEdit"));

        gridLayout_2->addWidget(m_pClippingPlaneNearEdit, 0, 1, 1, 1);

        m_pClippingPlaneFarEdit = new QLineEdit(groupBox);
        m_pClippingPlaneFarEdit->setObjectName(QStringLiteral("m_pClippingPlaneFarEdit"));

        gridLayout_2->addWidget(m_pClippingPlaneFarEdit, 1, 1, 1, 1);


        gridLayout->addLayout(gridLayout_2, 7, 1, 1, 1);

        label_24 = new QLabel(groupBox);
        label_24->setObjectName(QStringLiteral("label_24"));

        gridLayout->addWidget(label_24, 3, 0, 1, 1);

        m_pBackgroundTextureEdit = new QLineEdit(groupBox);
        m_pBackgroundTextureEdit->setObjectName(QStringLiteral("m_pBackgroundTextureEdit"));

        gridLayout->addWidget(m_pBackgroundTextureEdit, 3, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setStyleSheet(QStringLiteral(""));

        verticalLayout_3->addWidget(label_11);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        label_14 = new QLabel(groupBox);
        label_14->setObjectName(QStringLiteral("label_14"));

        gridLayout_3->addWidget(label_14, 0, 3, 1, 1);

        label_12 = new QLabel(groupBox);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout_3->addWidget(label_12, 0, 1, 1, 1);

        label_13 = new QLabel(groupBox);
        label_13->setObjectName(QStringLiteral("label_13"));

        gridLayout_3->addWidget(label_13, 1, 1, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QStringLiteral("label_15"));

        gridLayout_3->addWidget(label_15, 1, 3, 1, 1);

        m_pViewportXEdit = new QLineEdit(groupBox);
        m_pViewportXEdit->setObjectName(QStringLiteral("m_pViewportXEdit"));

        gridLayout_3->addWidget(m_pViewportXEdit, 0, 2, 1, 1);

        m_pViewportWEdit = new QLineEdit(groupBox);
        m_pViewportWEdit->setObjectName(QStringLiteral("m_pViewportWEdit"));

        gridLayout_3->addWidget(m_pViewportWEdit, 1, 2, 1, 1);

        m_pViewportYEdit = new QLineEdit(groupBox);
        m_pViewportYEdit->setObjectName(QStringLiteral("m_pViewportYEdit"));

        gridLayout_3->addWidget(m_pViewportYEdit, 0, 4, 1, 1);

        m_pViewportHEdit = new QLineEdit(groupBox);
        m_pViewportHEdit->setObjectName(QStringLiteral("m_pViewportHEdit"));

        gridLayout_3->addWidget(m_pViewportHEdit, 1, 4, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer, 0, 0, 1, 1);


        verticalLayout_3->addLayout(gridLayout_3);


        verticalLayout_2->addLayout(verticalLayout_3);


        verticalLayout->addWidget(groupBox);

        QWidget::setTabOrder(m_pIsMainCameraCB, m_pClearFlagCS);
        QWidget::setTabOrder(m_pClearFlagCS, m_pSolidColorButton);
        QWidget::setTabOrder(m_pSolidColorButton, m_pSolidColorPickButton);
        QWidget::setTabOrder(m_pSolidColorPickButton, m_pBackgroundTextureEdit);
        QWidget::setTabOrder(m_pBackgroundTextureEdit, m_pProjectionCB);
        QWidget::setTabOrder(m_pProjectionCB, m_pFieldOfViewSlider);
        QWidget::setTabOrder(m_pFieldOfViewSlider, m_pFieldOfViewEdit);
        QWidget::setTabOrder(m_pFieldOfViewEdit, m_pOrthographicSizeEdit);
        QWidget::setTabOrder(m_pOrthographicSizeEdit, m_pClippingPlaneNearEdit);
        QWidget::setTabOrder(m_pClippingPlaneNearEdit, m_pClippingPlaneFarEdit);
        QWidget::setTabOrder(m_pClippingPlaneFarEdit, m_pOcclusionCullingCB);
        QWidget::setTabOrder(m_pOcclusionCullingCB, m_pClearDepthEdit);
        QWidget::setTabOrder(m_pClearDepthEdit, m_pCameraModeCB);
        QWidget::setTabOrder(m_pCameraModeCB, m_pShutterSpeedEdit);
        QWidget::setTabOrder(m_pShutterSpeedEdit, m_pApertureEdit);
        QWidget::setTabOrder(m_pApertureEdit, m_pISOEdit);
        QWidget::setTabOrder(m_pISOEdit, m_pECEdit);
        QWidget::setTabOrder(m_pECEdit, m_pViewportXEdit);
        QWidget::setTabOrder(m_pViewportXEdit, m_pViewportYEdit);
        QWidget::setTabOrder(m_pViewportYEdit, m_pViewportWEdit);
        QWidget::setTabOrder(m_pViewportWEdit, m_pViewportHEdit);

        retranslateUi(InspectorCamera);
        QObject::connect(m_pIsMainCameraCB, SIGNAL(clicked()), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pSolidColorPickButton, SIGNAL(clicked()), InspectorCamera, SLOT(pickColorFromDialog()));
        QObject::connect(m_pFieldOfViewSlider, SIGNAL(valueChanged(int)), InspectorCamera, SLOT(fieldOfViewValueChanged(int)));
        QObject::connect(m_pClearFlagCS, SIGNAL(currentIndexChanged(int)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pSolidColorButton, SIGNAL(clicked()), InspectorCamera, SLOT(pickColorFromDialog()));
        QObject::connect(m_pBackgroundTextureEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pProjectionCB, SIGNAL(currentIndexChanged(int)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pOrthographicSizeEdit, SIGNAL(editingFinished()), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pClippingPlaneNearEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pClippingPlaneFarEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pOcclusionCullingCB, SIGNAL(clicked()), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pClearDepthEdit, SIGNAL(textChanged(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pCameraModeCB, SIGNAL(currentIndexChanged(int)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pShutterSpeedEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pApertureEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pISOEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pECEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pViewportXEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pViewportYEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pViewportWEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pViewportHEdit, SIGNAL(textEdited(QString)), InspectorCamera, SLOT(valueChanged()));
        QObject::connect(m_pFieldOfViewEdit, SIGNAL(textChanged(QString)), InspectorCamera, SLOT(valueChanged()));

        QMetaObject::connectSlotsByName(InspectorCamera);
    } // setupUi

    void retranslateUi(QWidget *InspectorCamera)
    {
        InspectorCamera->setWindowTitle(QApplication::translate("InspectorCamera", "Form", 0));
        groupBox->setTitle(QApplication::translate("InspectorCamera", "Camera", 0));
        m_pECEdit->setText(QApplication::translate("InspectorCamera", "0", 0));
        m_pOcclusionCullingCB->setText(QString());
        m_pISOEdit->setText(QApplication::translate("InspectorCamera", "100", 0));
        label_16->setText(QApplication::translate("InspectorCamera", "Depth", 0));
        label_21->setText(QApplication::translate("InspectorCamera", "EC", 0));
        label_3->setText(QApplication::translate("InspectorCamera", "Projection", 0));
        label_5->setText(QApplication::translate("InspectorCamera", "Clipping Planes", 0));
        label_2->setText(QApplication::translate("InspectorCamera", "Solid Color", 0));
        m_pSolidColorButton->setText(QString());
        m_pSolidColorPickButton->setText(QApplication::translate("InspectorCamera", "Pick", 0));
        m_pClearFlagCS->clear();
        m_pClearFlagCS->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Skybox", 0)
         << QApplication::translate("InspectorCamera", "Texture", 0)
         << QApplication::translate("InspectorCamera", "Solid Color", 0)
         << QApplication::translate("InspectorCamera", "No clear", 0)
        );
        m_pClearDepthEdit->setText(QApplication::translate("InspectorCamera", "-1", 0));
        m_pProjectionCB->clear();
        m_pProjectionCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Perspective", 0)
         << QApplication::translate("InspectorCamera", "Orthographic", 0)
        );
        label->setText(QApplication::translate("InspectorCamera", "Clear Flags", 0));
        label_8->setText(QApplication::translate("InspectorCamera", "Occlusion Culling", 0));
        label_9->setText(QApplication::translate("InspectorCamera", "Main Camera", 0));
        m_pIsMainCameraCB->setText(QString());
        label_10->setText(QApplication::translate("InspectorCamera", "Size", 0));
        m_pCameraModeCB->clear();
        m_pCameraModeCB->insertItems(0, QStringList()
         << QApplication::translate("InspectorCamera", "Auto", 0)
         << QApplication::translate("InspectorCamera", "Manual", 0)
        );
        m_pFieldOfViewEdit->setText(QApplication::translate("InspectorCamera", "60", 0));
        label_4->setText(QApplication::translate("InspectorCamera", "Field of View", 0));
        label_17->setText(QApplication::translate("InspectorCamera", "Camera Mode", 0));
        label_19->setText(QApplication::translate("InspectorCamera", "Aperture", 0));
        m_pApertureEdit->setText(QApplication::translate("InspectorCamera", "16", 0));
        label_23->setText(QApplication::translate("InspectorCamera", "flops", 0));
        label_20->setText(QApplication::translate("InspectorCamera", "ISO", 0));
        m_pShutterSpeedEdit->setText(QApplication::translate("InspectorCamera", "0.01", 0));
        label_22->setText(QApplication::translate("InspectorCamera", "seconds", 0));
        label_18->setText(QApplication::translate("InspectorCamera", "Shutter Speed", 0));
        label_6->setText(QApplication::translate("InspectorCamera", "Near", 0));
        label_7->setText(QApplication::translate("InspectorCamera", "Far", 0));
        m_pClippingPlaneNearEdit->setText(QApplication::translate("InspectorCamera", "0.3", 0));
        m_pClippingPlaneFarEdit->setText(QApplication::translate("InspectorCamera", "1024", 0));
        label_24->setText(QApplication::translate("InspectorCamera", "Backround Texture", 0));
        label_11->setText(QApplication::translate("InspectorCamera", "Viewport Rect", 0));
        label_14->setText(QApplication::translate("InspectorCamera", "Y", 0));
        label_12->setText(QApplication::translate("InspectorCamera", "X", 0));
        label_13->setText(QApplication::translate("InspectorCamera", "W", 0));
        label_15->setText(QApplication::translate("InspectorCamera", "H", 0));
        m_pViewportXEdit->setText(QApplication::translate("InspectorCamera", "0", 0));
        m_pViewportWEdit->setText(QApplication::translate("InspectorCamera", "1", 0));
        m_pViewportYEdit->setText(QApplication::translate("InspectorCamera", "0", 0));
        m_pViewportHEdit->setText(QApplication::translate("InspectorCamera", "1", 0));
    } // retranslateUi

};

namespace Ui {
    class InspectorCamera: public Ui_InspectorCamera {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDIT_INSPECTOR_CAMERA_H
