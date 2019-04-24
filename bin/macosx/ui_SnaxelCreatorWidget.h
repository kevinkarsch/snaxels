/********************************************************************************
** Form generated from reading UI file 'SnaxelCreatorWidget.ui'
**
** Created: Thu Jun 30 00:50:35 2011
**      by: Qt User Interface Compiler version 4.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SNAXELCREATORWIDGET_H
#define UI_SNAXELCREATORWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SnaxelCreatorWidgetUI
{
public:
    QPushButton *createButton;
    QSlider *goalNVSlider;
    QDoubleSpinBox *goalNVSpinBox;
    QLabel *label;

    void setupUi(QWidget *SnaxelCreatorWidgetUI)
    {
        if (SnaxelCreatorWidgetUI->objectName().isEmpty())
            SnaxelCreatorWidgetUI->setObjectName(QString::fromUtf8("SnaxelCreatorWidgetUI"));
        SnaxelCreatorWidgetUI->resize(347, 103);
        createButton = new QPushButton(SnaxelCreatorWidgetUI);
        createButton->setObjectName(QString::fromUtf8("createButton"));
        createButton->setGeometry(QRect(100, 50, 131, 41));
        goalNVSlider = new QSlider(SnaxelCreatorWidgetUI);
        goalNVSlider->setObjectName(QString::fromUtf8("goalNVSlider"));
        goalNVSlider->setGeometry(QRect(80, 10, 160, 22));
        goalNVSlider->setMinimum(0);
        goalNVSlider->setMaximum(100);
        goalNVSlider->setPageStep(10);
        goalNVSlider->setValue(0);
        goalNVSlider->setOrientation(Qt::Horizontal);
        goalNVSlider->setInvertedAppearance(false);
        goalNVSpinBox = new QDoubleSpinBox(SnaxelCreatorWidgetUI);
        goalNVSpinBox->setObjectName(QString::fromUtf8("goalNVSpinBox"));
        goalNVSpinBox->setGeometry(QRect(250, 10, 62, 25));
        goalNVSpinBox->setMaximum(1);
        goalNVSpinBox->setSingleStep(0.01);
        label = new QLabel(SnaxelCreatorWidgetUI);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 10, 71, 16));

        retranslateUi(SnaxelCreatorWidgetUI);

        QMetaObject::connectSlotsByName(SnaxelCreatorWidgetUI);
    } // setupUi

    void retranslateUi(QWidget *SnaxelCreatorWidgetUI)
    {
        SnaxelCreatorWidgetUI->setWindowTitle(QApplication::translate("SnaxelCreatorWidgetUI", "Form", 0, QApplication::UnicodeUTF8));
        createButton->setText(QApplication::translate("SnaxelCreatorWidgetUI", "Create front", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("SnaxelCreatorWidgetUI", "Goal N*V: ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SnaxelCreatorWidgetUI: public Ui_SnaxelCreatorWidgetUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SNAXELCREATORWIDGET_H
