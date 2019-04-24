/*
 * GLWindow.h
 * Display window for the QT OpenGL widget
 */

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "ui_GLWindow.h"

#include "GLWidget.h"

class GLWindow : public QMainWindow, private Ui::GLWindowUI
{
    Q_OBJECT

public:
    GLWindow( QWidget *parent = 0 );
	GLWidget* getGlWidget() { return glWidget; };

private slots:
	void openFile();
	void addFront();
	void removeLastFront();
	void toggleTimer();
	void toggleRenderFronts();
	void increaseGoalNV();
	void decreaseGoalNV();
	void exportPoseProjections();
	void saveGLFrame();
	void saveSVGFrame();

	void toggleRecording();
	void updateSVG();
	void saveAnimation();
	void toggleRidgeMode();
};

#endif
