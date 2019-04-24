/*
 * GLWidget.h
 * OpenGL display widget
 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QTimer>
#include "Mesh.h"
#include <stdlib.h>
#ifdef __APPLE__
#include "glew.h"
#include <GLUT/glut.h>
#else
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include <QGLWidget>
#include "SnaxelEvolver.h"
#include "PlanarMapDetector.h"
#include "SVGCreator.h"
#include "SVGWidget.h"

class GLWidget : public QGLWidget
{
	Q_OBJECT

public:
	GLWidget(QWidget *parent = 0);
	~GLWidget();
	QSize minimumSizeHint() const { return QSize(50, 50); };
	QSize sizeHint() const { return QSize(400, 400); };
	inline Mesh *getMesh() { return mesh; };
	inline void setMesh(Mesh *m) { mesh = m; };
	
	//Finds and returns the current camera position (modelview coordinates)
	void getViewpoint(float *vp);
	
	void toggleDeformationTimer();
	
	void createEvolver(float *rgba, float ndotv, float *fixedViewpoint = NULL, Vertex* initVert = NULL);
	void removeLastEvolver();
	void initializeSnaxels();
	void clearSnaxels();
	
	vector<SnaxelEvolver*> *getEvolvers() { return &evolvers; };
	vector<SnaxelEvolver*> *getBackEvolvers() { return &backevolvers; };

	void toggleRenderFronts() { renderFronts = !renderFronts; };

	void rotateView(float theta, float phi); //Rotates the current modelview matrix

	//Display for svg
	SVGWidget *svgWidget;
	SVGCreator svgAnimation, planarMapSVG;
	bool recordingAnimation;

	Vertex* findNearestVertex(float x, float y);
	//Currently selected vertex
	Vertex* selectedVertex;

	bool ridgeModeOn;

	vector< vector< Vertex > > p_silhouette;
	
protected:
	//Child-inherited functions for controlling the display
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

private:
	//Pointer to a loaded mesh
	Mesh *mesh;
	//Snaxel evolver
	vector<SnaxelEvolver*> evolvers;
	vector<SnaxelEvolver*> backevolvers;
	//Planar map described by gpc_polygons
	vector<gpc_polygon>* planarMap;
	//Planar map detection class
	PlanarMapDetector pmapDetector;
	//Current viewpoint
	float currentViewpoint[3];
	
	//Snaxel deformation timer 
	QTimer deformationTimer;
	//Display parameters (user controlled)
	GLfloat cameraRotationX, cameraRotationY;
	GLfloat cameraTranslationX, cameraTranslationY, cameraTranslationZ;
	QPoint lastMousePosition;
	//Holds clock values to only allow GL updates at certain intervals
	double lastUpdate;
	//Display filled fronts
	bool renderFronts;
	//Show selection mode
	bool renderInSelectionMode;
	//Only update an evolver until it has converged (this keeps track of that)
	vector<SnaxelEvolver*> evolversThatNeedUpdates;
	vector<SnaxelEvolver*> backevolversThatNeedUpdates;

public slots:
	/* deformSnaxels() - Deforms all snaxels one timestep
	 * Procedure as follows: 
	 *  1) Deform all snaxels, removing snaxels which have less then 4 vertices
	 *  2) Split snaxels which are self-intersecting
	 *  3) Merge snaxels which have intersected
	 */
	void deformSnaxels(bool toEquillibruim = true);
};

#endif
