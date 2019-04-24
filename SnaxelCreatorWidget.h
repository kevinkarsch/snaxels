/*
 * SnaxelCreatorWidget.h
 * Modal window for creating new snaxel fronts
 */

#ifndef SNAXELCREATORWIDGET_H
#define SNAXELCREATORWIDGET_H
#include <QDialog>

#include "ui_SnaxelCreatorWidget.h"

#include "SnaxelCreatorWidget.h"
#include "GLWindow.h"

class SnaxelCreatorWidget : public QDialog, private Ui::SnaxelCreatorWidgetUI
{
    Q_OBJECT

public:
    SnaxelCreatorWidget( GLWindow *parent = 0 );
	void getParameters(float *nvmin);

private slots:
	void updateNVSpinBox();
	void updateNVSlider();
	void createSnaxels();
};

#endif
