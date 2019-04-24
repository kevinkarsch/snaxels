/********************************************************************************
** Form generated from reading UI file 'GLWindow.ui'
**
** Created: Wed Feb 1 18:34:14 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLWINDOW_H
#define UI_GLWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>
#include "GLWidget.h"
#include "SVGWidget.h"

QT_BEGIN_NAMESPACE

class Ui_GLWindowUI
{
public:
    QAction *actionOpen;
    QAction *actionAdd_front;
    QAction *actionToggle_deformation;
    QAction *actionRemove_last_front;
    QAction *actionRender_fronts;
    QAction *actionIncrease_goal_N_V;
    QAction *actionDecrease_goal_N_V;
    QAction *actionExport_all_pose_projections;
    QAction *actionExport_animation;
    QAction *actionSave_GL_frame;
    QAction *actionRidge_mode;
    QAction *actionSave_SVG_frame;
    QWidget *centralwidget;
    GLWidget *glWidget;
    SVGWidget *svgWidget;
    QPushButton *recordButton;
    QPlainTextEdit *defsText;
    QPlainTextEdit *cssText;
    QLabel *label;
    QLabel *label_2;
    QSlider *frameSlider;
    QListView *depthList;
    QLabel *label_3;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuSnaxels;
    QMenu *menuExperiments;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GLWindowUI)
    {
        if (GLWindowUI->objectName().isEmpty())
            GLWindowUI->setObjectName(QString::fromUtf8("GLWindowUI"));
        GLWindowUI->resize(1090, 788);
        GLWindowUI->setMinimumSize(QSize(1090, 788));
        GLWindowUI->setMaximumSize(QSize(1090, 788));
        GLWindowUI->setTabShape(QTabWidget::Rounded);
        actionOpen = new QAction(GLWindowUI);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionAdd_front = new QAction(GLWindowUI);
        actionAdd_front->setObjectName(QString::fromUtf8("actionAdd_front"));
        actionToggle_deformation = new QAction(GLWindowUI);
        actionToggle_deformation->setObjectName(QString::fromUtf8("actionToggle_deformation"));
        actionRemove_last_front = new QAction(GLWindowUI);
        actionRemove_last_front->setObjectName(QString::fromUtf8("actionRemove_last_front"));
        actionRender_fronts = new QAction(GLWindowUI);
        actionRender_fronts->setObjectName(QString::fromUtf8("actionRender_fronts"));
        actionRender_fronts->setCheckable(true);
        actionIncrease_goal_N_V = new QAction(GLWindowUI);
        actionIncrease_goal_N_V->setObjectName(QString::fromUtf8("actionIncrease_goal_N_V"));
        actionDecrease_goal_N_V = new QAction(GLWindowUI);
        actionDecrease_goal_N_V->setObjectName(QString::fromUtf8("actionDecrease_goal_N_V"));
        actionExport_all_pose_projections = new QAction(GLWindowUI);
        actionExport_all_pose_projections->setObjectName(QString::fromUtf8("actionExport_all_pose_projections"));
        actionExport_animation = new QAction(GLWindowUI);
        actionExport_animation->setObjectName(QString::fromUtf8("actionExport_animation"));
        actionSave_GL_frame = new QAction(GLWindowUI);
        actionSave_GL_frame->setObjectName(QString::fromUtf8("actionSave_GL_frame"));
        actionRidge_mode = new QAction(GLWindowUI);
        actionRidge_mode->setObjectName(QString::fromUtf8("actionRidge_mode"));
        actionSave_SVG_frame = new QAction(GLWindowUI);
        actionSave_SVG_frame->setObjectName(QString::fromUtf8("actionSave_SVG_frame"));
        centralwidget = new QWidget(GLWindowUI);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        glWidget = new GLWidget(centralwidget);
        glWidget->setObjectName(QString::fromUtf8("glWidget"));
        glWidget->setGeometry(QRect(9, 9, 536, 431));
        svgWidget = new SVGWidget(centralwidget);
        svgWidget->setObjectName(QString::fromUtf8("svgWidget"));
        svgWidget->setGeometry(QRect(551, 9, 535, 492));
        recordButton = new QPushButton(centralwidget);
        recordButton->setObjectName(QString::fromUtf8("recordButton"));
        recordButton->setGeometry(QRect(464, 450, 81, 51));
        recordButton->setStyleSheet(QString::fromUtf8("color: red"));
        recordButton->setCheckable(true);
        recordButton->setChecked(false);
        defsText = new QPlainTextEdit(centralwidget);
        defsText->setObjectName(QString::fromUtf8("defsText"));
        defsText->setGeometry(QRect(550, 525, 536, 216));
        defsText->setLineWrapMode(QPlainTextEdit::NoWrap);
        cssText = new QPlainTextEdit(centralwidget);
        cssText->setObjectName(QString::fromUtf8("cssText"));
        cssText->setGeometry(QRect(273, 525, 271, 216));
        cssText->setLineWrapMode(QPlainTextEdit::NoWrap);
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(550, 505, 536, 21));
        QFont font;
        font.setPointSize(12);
        font.setUnderline(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(275, 505, 261, 21));
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);
        frameSlider = new QSlider(centralwidget);
        frameSlider->setObjectName(QString::fromUtf8("frameSlider"));
        frameSlider->setGeometry(QRect(559, 480, 516, 19));
        frameSlider->setOrientation(Qt::Horizontal);
        frameSlider->setTickPosition(QSlider::TicksBelow);
        frameSlider->setTickInterval(24);
        depthList = new QListView(centralwidget);
        depthList->setObjectName(QString::fromUtf8("depthList"));
        depthList->setGeometry(QRect(5, 525, 261, 216));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(5, 505, 261, 21));
        label_3->setFont(font);
        label_3->setAlignment(Qt::AlignCenter);
        GLWindowUI->setCentralWidget(centralwidget);
        menubar = new QMenuBar(GLWindowUI);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1090, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuSnaxels = new QMenu(menubar);
        menuSnaxels->setObjectName(QString::fromUtf8("menuSnaxels"));
        menuExperiments = new QMenu(menubar);
        menuExperiments->setObjectName(QString::fromUtf8("menuExperiments"));
        GLWindowUI->setMenuBar(menubar);
        statusbar = new QStatusBar(GLWindowUI);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        GLWindowUI->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuSnaxels->menuAction());
        menubar->addAction(menuExperiments->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave_SVG_frame);
        menuFile->addAction(actionSave_GL_frame);
        menuFile->addAction(actionExport_animation);
        menuSnaxels->addAction(actionAdd_front);
        menuSnaxels->addAction(actionRemove_last_front);
        menuSnaxels->addSeparator();
        menuSnaxels->addAction(actionToggle_deformation);
        menuSnaxels->addSeparator();
        menuSnaxels->addAction(actionRender_fronts);
        menuSnaxels->addSeparator();
        menuSnaxels->addAction(actionIncrease_goal_N_V);
        menuSnaxels->addAction(actionDecrease_goal_N_V);
        menuExperiments->addAction(actionExport_all_pose_projections);
        menuExperiments->addAction(actionRidge_mode);

        retranslateUi(GLWindowUI);

        QMetaObject::connectSlotsByName(GLWindowUI);
    } // setupUi

    void retranslateUi(QMainWindow *GLWindowUI)
    {
        GLWindowUI->setWindowTitle(QString());
        actionOpen->setText(QApplication::translate("GLWindowUI", "Open", 0, QApplication::UnicodeUTF8));
        actionOpen->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionAdd_front->setText(QApplication::translate("GLWindowUI", "Add front", 0, QApplication::UnicodeUTF8));
        actionAdd_front->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionToggle_deformation->setText(QApplication::translate("GLWindowUI", "Toggle deformation", 0, QApplication::UnicodeUTF8));
        actionToggle_deformation->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+D", 0, QApplication::UnicodeUTF8));
        actionRemove_last_front->setText(QApplication::translate("GLWindowUI", "Remove last front", 0, QApplication::UnicodeUTF8));
        actionRemove_last_front->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+Shift+R", 0, QApplication::UnicodeUTF8));
        actionRender_fronts->setText(QApplication::translate("GLWindowUI", "Render filled fronts", 0, QApplication::UnicodeUTF8));
        actionRender_fronts->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+F", 0, QApplication::UnicodeUTF8));
        actionIncrease_goal_N_V->setText(QApplication::translate("GLWindowUI", "Increase goal N*V", 0, QApplication::UnicodeUTF8));
        actionIncrease_goal_N_V->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+=", 0, QApplication::UnicodeUTF8));
        actionDecrease_goal_N_V->setText(QApplication::translate("GLWindowUI", "Decrease goal N*V", 0, QApplication::UnicodeUTF8));
        actionDecrease_goal_N_V->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+-", 0, QApplication::UnicodeUTF8));
        actionExport_all_pose_projections->setText(QApplication::translate("GLWindowUI", "Export all pose projections", 0, QApplication::UnicodeUTF8));
        actionExport_animation->setText(QApplication::translate("GLWindowUI", "Export animation", 0, QApplication::UnicodeUTF8));
        actionExport_animation->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+Shift+S", 0, QApplication::UnicodeUTF8));
        actionSave_GL_frame->setText(QApplication::translate("GLWindowUI", "Save GL frame", 0, QApplication::UnicodeUTF8));
        actionSave_GL_frame->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+Alt+S", 0, QApplication::UnicodeUTF8));
        actionRidge_mode->setText(QApplication::translate("GLWindowUI", "Ridge mode", 0, QApplication::UnicodeUTF8));
        actionSave_SVG_frame->setText(QApplication::translate("GLWindowUI", "Save SVG frame", 0, QApplication::UnicodeUTF8));
        actionSave_SVG_frame->setShortcut(QApplication::translate("GLWindowUI", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        recordButton->setText(QApplication::translate("GLWindowUI", "Record\n"
"Animation", 0, QApplication::UnicodeUTF8));
        defsText->setPlainText(QApplication::translate("GLWindowUI", "<linearGradient id=\"lingrad\" x1=\"0%\" y1=\"0%\" x2=\"0%\" y2=\"100%\">\n"
"	<stop offset=\"0%\" style=\"stop-color:rgb(255,0,0);stop-opacity:1\"/>\n"
"	<stop offset=\"100%\" style=\"stop-color:rgb(255,255,0);stop-opacity:1\"/>\n"
"</linearGradient>\n"
"\n"
"<radialGradient id=\"radgrad\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"50%\" fy=\"50%\">\n"
"	<stop offset=\"0%\" style=\"stop-color:rgb(200,200,200); stop-opacity:0\"/>\n"
"	<stop offset=\"100%\" style=\"stop-color:rgb(0,255,255); stop-opacity:1\"/>\n"
"</radialGradient>\n"
"\n"
"<pattern id=\"scales\" patternUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"25\" height=\"25\">\n"
"	<g style=\"fill:none; stroke:url(#radgrad); stroke-width:1\">\n"
"		<path d=\"M0 0 C12,12 20,24 0,24\"/>\n"
"	</g>\n"
"</pattern>\n"
"\n"
"<pattern id=\"hatch\" patternUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"25\" height=\"25\">\n"
"	<g style=\"fill:none; stroke:black; stroke-width:1\">\n"
"		<path d=\"M0 0 L12,12 L0,24\"/>\n"
"	</g>\n"
"</pattern>\n"
"\n"
"<marker id=\""
                        "marker\" viewBox=\"0 0 20 20\" refX=\"0\" refY=\"0\" markerWidth=\"4\" markerHeight=\"4\" stroke=\"blue\" stroke-width=\"2\" fill=\"none\" orient=\"auto\">\n"
"	<circle cx=\"10\" cy=\"10\" r=\"10\" stroke=\"black\" stroke-width=\"2\" fill=\"red\"/>\n"
"</marker>", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("GLWindowUI", "Definitions", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("GLWindowUI", "CSS", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("GLWindowUI", "Depth ordering", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("GLWindowUI", "File", 0, QApplication::UnicodeUTF8));
        menuSnaxels->setTitle(QApplication::translate("GLWindowUI", "Snaxels", 0, QApplication::UnicodeUTF8));
        menuExperiments->setTitle(QApplication::translate("GLWindowUI", "Experiments", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class GLWindowUI: public Ui_GLWindowUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLWINDOW_H
