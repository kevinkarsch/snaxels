#include "GLWidget.h"
#include <QtGui>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include "glew.h"
#else
#include <GL/glew.h>
#endif
#include <QtOpenGL>

#include "ArcBall.h"

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent) {
	cameraRotationX = 0.f;
	cameraRotationY = 0.f;

	cameraTranslationX = 0.f;
	cameraTranslationY = 0.f;
	cameraTranslationZ = -3.f;
	
	lastUpdate = 0;
	renderFronts = false;
	renderInSelectionMode = false;
	selectedVertex = NULL;
	
	mesh = new Mesh();
	planarMap = NULL;
	
	recordingAnimation = false;

	setFocusPolicy(Qt::StrongFocus);

	svgAnimation = SVGCreator(535,492);
	planarMapSVG = SVGCreator(535,492);

	ridgeModeOn = false;

	//Connect the timer to the slot function
//Uncomment below to use timed updates (otherwise updates only happen on camera movement)
	//connect(&deformationTimer, SIGNAL(timeout()), this, SLOT(deformSnaxels()));
}

GLWidget::~GLWidget() {
	clearSnaxels();
	delete mesh;
}

Matrix3fT rot,baserot;
Matrix4fT xform = {1.,0.,0.,0., 0.,1.,0.,0., 0.,0.,1.,0., 0.,0.,0.,1.};
ArcBallT arcball(100,100);

void GLWidget::initializeGL() {
	//GLEW stuff
	GLenum err = glewInit();
	if( GLEW_OK != err ) 
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	if( !GLEW_ARB_occlusion_query ) 
		fprintf(stderr, "Error: ARB_occlusion_query not supported\n");

	glClearColor( 1, 1, 1, 1 );
	glClearDepth( 1.0f );
	glShadeModel( GL_SMOOTH );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glEnable( GL_NORMALIZE );
	
	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_specular[] = { 0.8, 0.8, 0.8, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
	GLfloat lm_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat light_position[] = { 0.f, 0.f, 1.f, 0.f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix3fSetIdentity(&rot);
	Matrix3fSetIdentity(&baserot);
	Matrix4fSetRotationFromMatrix3f(&xform,&baserot);
}

void GLWidget::resizeGL(int width, int height) {
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, float(width)/float(height), .1, 100);
	glMatrixMode(GL_MODELVIEW);

	arcball.setBounds((GLfloat) width, (GLfloat) height);
}

void GLWidget::paintGL() {
	makeCurrent();
	if( glewGetExtension("GL_ARB_multisample") ) glEnable(GL_MULTISAMPLE_ARB);
	else glDisable(GL_MULTISAMPLE_ARB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glPushMatrix();
	glLoadIdentity();
	glTranslatef( cameraTranslationX, cameraTranslationY, cameraTranslationZ );
//	glRotatef(cameraRotationX, 1.0, 0.0, 0.0);
//	glRotatef(cameraRotationY, 0.0, 1.0, 0.0);
	glMultMatrixf(xform.M);
	
	float color[4] = {0.8f, 0.8f, 0.8f, 1.f};
	glColor4fv( color );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, color );

	//glColorMask(false,false,false,false);
	mesh->render(GL_FRONT_AND_BACK, GL_FILL, SOFT_NORMALS, renderFronts, renderInSelectionMode);
	//glColorMask(true,true,true,true);
	
	for(unsigned i = 0; i < evolvers.size(); i++) evolvers[i]->renderSnaxels();
	for(unsigned i = 0; i < backevolvers.size(); i++) backevolvers[i]->renderSnaxels();
	
	//Render valley/ridge edges
	/*
	if(ridgeModeOn) { 
		glPointSize(10.f);
		glDisable(GL_LIGHTING);
		glColor3f(0,0,1);
		glBegin(GL_POINTS);
		for(unsigned i = 0; i < mesh->getEdges().size(); i++)
			if(mesh->getEdges()[i]->ridge)
				glVertex3fv(mesh->getEdges()[i]->ridge_p);
		glEnd();
		glPointSize(1.f);
	}
	*/
	
	//glClear(GL_DEPTH_BUFFER_BIT);
	//Render 3D silhouette (kalnins)
	/*
	glLineWidth(4.f);
	glPointSize(10.f);
	glDisable(GL_LIGHTING);
	float vp[3]; 
	getViewpoint(&vp[0]);
	for(unsigned i = 0; i < p_silhouette.size(); i++) {
		if( p_silhouette[i].size() < 2 ) continue;
		glColor3f(.8,.2,.7);
		glBegin(GL_POLYGON);
		
		Vertex *v1 = &p_silhouette[i].front();
		Vertex *v2 = &p_silhouette[i].back();
		float mdp[3]={0.5f*(v1->p[0]+v2->p[0]), 0.5f*(v1->p[1]+v2->p[1]), 0.5f*(v1->p[2]+v2->p[2])};
		float vv[3]={ mdp[0]-vp[0], mdp[1]-vp[1], mdp[2]-vp[2] };
		normalize(vv);
		float dir[3]={v2->p[0]-v1->p[0], v2->p[1]-v1->p[1], v2->p[2]-v1->p[2]};
		normalize(dir);
		float mdn[3]; 
		cross(vv, dir, mdn);
		normalize(mdn);
		float sizecurve = 0.03f;
		float midp[3] = {mdp[0]+mdn[0]*sizecurve, mdp[1]+mdn[1]*sizecurve, mdp[2]+mdn[2]*sizecurve};
		for(float t=0.f; t<=1.f; t+= 0.5f) { //Quadratic interp
			float c1 = (1.f-t)*(1.f-t), c2 = 2.f*(1.f-t)*t, c3 = t*t;
			glVertex3f(c1*v1->p[0] + c2*midp[0] + c3*v2->p[0],
					   c1*v1->p[1] + c2*midp[1] + c3*v2->p[1],
					   c1*v1->p[2] + c2*midp[2] + c3*v2->p[2]);
		}
		
		//for(unsigned j=0; j < p_silhouette[i].size(); j++)
		//	glVertex3fv( p_silhouette[i][j].p );
		glEnd();
	}
	glLineWidth(3.f);
	glPointSize(1.f);
	*/

	glClear(GL_DEPTH_BUFFER_BIT);
	//Renders dashed lines for occluded visual contours
	for(unsigned i = 0; i < evolvers.size(); i++) evolvers[i]->renderSnaxels(true);
	for(unsigned i = 0; i < backevolvers.size(); i++) backevolvers[i]->renderSnaxels(true);

	//Display vertices where snaxels were initialized
	/*
	glPointSize(10.f);
	glDisable(GL_LIGHTING);
	glBegin(GL_POINTS);
	for(int i=0; i<mesh->getVertices().size(); i++) {
		if(mesh->getVertices()[i]->initialize) {
			float t = mesh->getVertices()[i]->ndotv > 0 ? 1 : 0;
			glColor3f(t,0,1-t);
			glVertex3fv(mesh->getVertices()[i]->p);
		}
	}
	glEnd();
	*/

	//Display selected vertex
	if(selectedVertex) {
		glPointSize(10.f); glColor3f(0,1,1); 
		glBegin(GL_POINTS); glVertex3fv(selectedVertex->p); glEnd();
	}



	//glPopMatrix();
	
	//glClear(GL_DEPTH_BUFFER_BIT);
	//pmapDetector.renderPlanarMap(planarMap);

	resizeGL(width(), height()); // Reset viewport/projection
}

Vertex* GLWidget::findNearestVertex(float x, float y) {
	makeCurrent();
	glDisable(GL_MULTISAMPLE_ARB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef( cameraTranslationX, cameraTranslationY, cameraTranslationZ );
	glMultMatrixf(xform.M);
	mesh->render(GL_FRONT, GL_POINTS, NO_NORMALS, false, true);
	unsigned char pixel[3];
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, height()-y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel); //Assumes glClearColor(1,1,1,1);
	unsigned long idx = 65536*pixel[0] + 256*pixel[1] + pixel[2];
	Vertex *v = NULL;
	if(idx < mesh->getVertices().size())
		v = mesh->getVertices()[idx];
	return v;
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
	if( (event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier) ) {
		selectedVertex = findNearestVertex(event->x(), event->y());
		updateGL();
	} else {
		lastMousePosition = event->pos();
		baserot = rot;
		arcball.click(event->x(), event->y());
	}
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
	bool accepted = false;
	int dx = event->x() - lastMousePosition.x();
	int dy = event->y() - lastMousePosition.y();

	if (event->buttons() & Qt::LeftButton) {
		Quat4fT quat;
		arcball.drag(event->x(),event->y(),&quat);
		Matrix3fSetRotationFromQuat4f(&rot,&quat);
		Matrix3fMulMatrix3f(&rot,&baserot);
		Matrix4fSetRotationFromMatrix3f(&xform,&rot);
		accepted = true;
	}

	//if( event->buttons() & Qt::LeftButton ) {
	//	cameraRotationX += 0.025f * GLfloat(dy);
	//	cameraRotationY += 0.025f * GLfloat(dx);
	//	accepted = true;
	//}

	if( event->buttons() & Qt::RightButton ) {
		cameraTranslationX += 0.001f * GLfloat(dx);
		cameraTranslationY -= 0.001f * GLfloat(dy);
		accepted = true;
	}

	if( accepted ) {
		//Initialize on view change (only in uninitialized regions)
		if( deformationTimer.isActive() ) {
			//initializeSnaxels();
		}
	double startTime = getTime();
		initializeSnaxels();
		lastMousePosition = event->pos();
			deformSnaxels();
			updateGL();
		event->accept();
	}
}

void GLWidget::wheelEvent(QWheelEvent *event) {
	if( event->orientation() == Qt::Vertical ) {
		cameraTranslationZ += 0.0008f * GLfloat(event->delta());
			deformSnaxels();
			updateGL();
		event->accept();
	}
}

float rottheta_=0, rotphi_=0;
void GLWidget::keyPressEvent(QKeyEvent *event) {
	float angle = 1.0 * 3.14157/180.0;
	switch( event->key() ) {
		case Qt::Key_D:
			deformSnaxels(false); //Deform snaxels forward one timestep
			event->accept();
			break;
		case Qt::Key_Left:
			rottheta_ += angle;
			rotateView(rottheta_, rotphi_);
			initializeSnaxels();
			deformSnaxels();
			//updateGL();
			event->accept();
			break;
		case Qt::Key_Right:
			rottheta_ -= angle;
			rotateView(rottheta_, rotphi_);
			initializeSnaxels();
			deformSnaxels();
			//updateGL();
			event->accept();
			break;
		case Qt::Key_Up:
			rotphi_ -= angle;
			rotateView(rottheta_, rotphi_);
			initializeSnaxels();
			deformSnaxels();
			event->accept();
			break;
		case Qt::Key_Down:
			rotphi_ += angle;
			rotateView(rottheta_, rotphi_);
			initializeSnaxels();
			deformSnaxels();
			event->accept();
			break;
		case Qt::Key_Plus:
			if(selectedVertex) {
				evolvers.back()->initializeAroundSingleVertex( selectedVertex );
				//backevolvers.back()->initializeAroundSingleVertex( selectedVertex );
			}
			updateGL();
			event->accept();
			break;
		case Qt::Key_Shift:
			renderInSelectionMode = true;
			updateGL();
			event->accept();
			break;
	}
}
void GLWidget::keyReleaseEvent(QKeyEvent *event) {
	switch( event->key() ) {
		case Qt::Key_Shift:
			renderInSelectionMode = false;
			updateGL();
			event->accept();
			break;
	}
}

void GLWidget::rotateView(float theta, float phi) { //Rotates the current modelview matrix
	Matrix3fT theta_rot, phi_rot, all_rot;
	Matrix3fSetIdentity(&theta_rot);
	theta_rot.s.M00 = cos(theta); theta_rot.s.M02 = sin(theta); theta_rot.s.M20 = -sin(theta); theta_rot.s.M22 = cos(theta);
	Matrix3fSetIdentity(&phi_rot);
	phi_rot.s.M11 = cos(phi); phi_rot.s.M12 = -sin(phi); phi_rot.s.M21 = sin(phi); phi_rot.s.M22 = cos(phi);
	//Rotate by theta, then phi
	Matrix3fSetIdentity(&all_rot);
	Matrix3fMulMatrix3f(&all_rot,&phi_rot);
	Matrix3fMulMatrix3f(&all_rot,&theta_rot);
	Matrix3fMulMatrix3f(&all_rot,&baserot);
	Matrix4fSetRotationFromMatrix3f(&xform,&all_rot);
}

void GLWidget::toggleDeformationTimer() {
	if( deformationTimer.isActive() )
		deformationTimer.stop();
	else 
		deformationTimer.start();
}

void GLWidget::createEvolver(float *rgba, float ndotv, float *fixedViewpoint, Vertex* initVert) {
	evolvers.push_back( new SnaxelEvolver(mesh, rgba, ndotv, fixedViewpoint) );
	evolversThatNeedUpdates.push_back(evolvers.back());
	if( evolvers.size() == 1 )
		initializeSnaxels();
	else if(initVert == NULL) 
		evolvers.back()->initializeSnaxelsPersp( fixedViewpoint );
	else
		evolvers.back()->initializeAroundSingleVertex( initVert );

/*
	backevolvers.push_back( new SnaxelEvolver(mesh, rgba, ndotv, fixedViewpoint, true) );
	backevolversThatNeedUpdates.push_back(backevolvers.back());
	if( backevolvers.size() == 1 )
		initializeSnaxels();
	else if(initVert == NULL)
		backevolvers.back()->initializeSnaxelsPersp( fixedViewpoint );
	else
		backevolvers.back()->initializeAroundSingleVertex( initVert );
*/
}

void GLWidget::getViewpoint(float *vp) {
	float vp_hc[4] = {-cameraTranslationX, -cameraTranslationY, -cameraTranslationZ, 1.f};
	for(int i = 0; i < 3; i++) {
		vp[i] = xform.M[i*4+0]*vp_hc[0] + xform.M[i*4+1]*vp_hc[1] + 
					  xform.M[i*4+2]*vp_hc[2] + xform.M[i*4+3]*vp_hc[3];
	}
}

//This only initializes the 0'th evolver (the only dynamic one)
void GLWidget::initializeSnaxels() {
	double startTime = getTime();
	getViewpoint(&currentViewpoint[0]);
	if( !evolvers.empty() )
		evolvers[0]->initializeSnaxelsPersp( currentViewpoint );
	//if(!backevolvers.empty())
	//	backevolvers[0]->initializeSnaxelsPersp( currentViewpoint );
	
	//printf("Initialization: %15.10fs\n", getTime()-startTime);
	updateGL();
}

int numUpdates = 0; float snaxelUpdateTime = 0; float pmapUpdateTime = 0; float svgUpdateTime = 0;
void GLWidget::deformSnaxels(bool toEquillibruim) {
	//if( evolvers.empty() || backevolvers.empty() ) return;
	if( evolvers.empty() ) return;
	
	//Calculate projection matrix
	float proj[16]; glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glLoadMatrixf( proj ); //Let opengl do the proj*modelview calculation
	glTranslatef( cameraTranslationX, cameraTranslationY, cameraTranslationZ );
	glMultMatrixf( xform.M );
	glMultMatrixf( mesh->getTform() );
	glGetFloatv(GL_MODELVIEW_MATRIX, proj); 
	
	double startTime = getTime();

	//Get current viewpoint
	getViewpoint(&currentViewpoint[0]);

	//Records animation frames to be linked together for svg animation (different from static animation)
	//This function also initialize parents of the snaxels as themselves 
	//    (updated during evolution to get frame-to-frame correspondences)
	//BackEvolvers not implemented!!
	if(recordingAnimation)
		svgAnimation.recordDynamicFrame( &evolversThatNeedUpdates, mesh );

	//Evolve the fronts
	if( toEquillibruim ) {
		float E;
		float stopThreshold = 1e-8;
		int itrs = 0;
		do {
			E = 0;
			vector<SnaxelEvolver*> cumEvolvForPlanarMaps = evolversThatNeedUpdates;
			reverse(cumEvolvForPlanarMaps.begin(), cumEvolvForPlanarMaps.end());
			for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++) {
				float tmpE = evolversThatNeedUpdates[i]->evolve(0.01f, 0.5f, currentViewpoint, true, &evolversThatNeedUpdates); // true => verbose/debug
				cumEvolvForPlanarMaps.pop_back();
				if(i != 0 && tmpE < stopThreshold) {}
					//evolversThatNeedUpdates.erase(evolversThatNeedUpdates.begin() + i--);
				else
					E += tmpE;
			}
			vector<SnaxelEvolver*> cumBackEvolvForPlanarMaps = backevolversThatNeedUpdates;
			reverse(cumBackEvolvForPlanarMaps.begin(), cumBackEvolvForPlanarMaps.end());
			for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++) {
				float tmpE = backevolversThatNeedUpdates[i]->evolve(0.01f, 0.5f, currentViewpoint, true, &backevolversThatNeedUpdates); // true => verbose/debug
				cumBackEvolvForPlanarMaps.pop_back();
				if(i != 0 && tmpE < stopThreshold) {}
					//backevolversThatNeedUpdates.erase(backevolversThatNeedUpdates.begin() + i--);
				else
					E += tmpE;
			}
			E /= float(evolversThatNeedUpdates.size() + backevolversThatNeedUpdates.size());
			if( ++itrs > 1000 ) stopThreshold *= 10.f;
		} while( E > stopThreshold );
	} else {
		vector<SnaxelEvolver*> cumEvolvForPlanarMaps = evolversThatNeedUpdates;
		reverse(cumEvolvForPlanarMaps.begin(), cumEvolvForPlanarMaps.end());
		for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++) {
			float tmpE = evolversThatNeedUpdates[i]->evolve(0.01f, 0.5f, currentViewpoint, true, &evolversThatNeedUpdates); // true => verbose/debug
			cumEvolvForPlanarMaps.pop_back();
		}	
		vector<SnaxelEvolver*> cumBackEvolvForPlanarMaps = backevolversThatNeedUpdates;
		reverse(cumBackEvolvForPlanarMaps.begin(), cumBackEvolvForPlanarMaps.end());
		for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++) {
			float tmpE = backevolversThatNeedUpdates[i]->evolve(0.01f, 0.5f, currentViewpoint, true, &backevolversThatNeedUpdates); // true => verbose/debug
			cumBackEvolvForPlanarMaps.pop_back();
		}
	}

	//Remove occluded fronts
	//BackEvolvers not implemented!!
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//mesh->render(GL_FRONT_AND_BACK, GL_FILL, NO_NORMALS, renderFronts);
	//for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++)
	//	evolversThatNeedUpdates[i]->removeOcclusions(0.01f, 0.5f, currentViewpoint, true);

	//Find projected snaxel positions
	for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++)
		evolversThatNeedUpdates[i]->projectSnaxels(proj, float(width()), float(height()) );
	for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++)
		backevolversThatNeedUpdates[i]->projectSnaxels(proj, float(width()), float(height()) );
	
	/*
	//Calculate each front's depth (distance to camera)
	for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++)
		evolversThatNeedUpdates[i]->calculateFrontDepthAndSort(proj);
	for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++)
		backevolversThatNeedUpdates[i]->calculateFrontDepthAndSort(proj);
	*/
		
	//Update head snaxels for contour coherence
	for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++)
		evolversThatNeedUpdates[i]->makeHeadsCoherent();
	for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++)
		backevolversThatNeedUpdates[i]->makeHeadsCoherent();
	
	//Stop timer
	snaxelUpdateTime += getTime()-startTime;
	
	//Update view (calculates snaxel occlusions also)
	updateGL();
	
	/*
	//Calculate whether the front consitutes a hole in the planar map
	for(unsigned i = 0; i < evolversThatNeedUpdates.size(); i++)
		evolversThatNeedUpdates[i]->calculateHoleFronts(proj);
	for(unsigned i = 0; i < backevolversThatNeedUpdates.size(); i++)
		backevolversThatNeedUpdates[i]->calculateHoleFronts(proj);
	*/

	//Find planar maps 
	//BackEvolvers not implemented!!
	startTime = getTime();
	/*
	pmapDetector.freeMap(planarMap);
	vector<SnaxelEvolver*> *pmapEvolvers;
	planarMap = pmapDetector.findPlanarMap(evolversThatNeedUpdates, proj, &pmapEvolvers, 535,492 );
	planarMapSVG.recordStaticFrame( pmapEvolvers );
	planarMapSVG.setCSS( planarMapSVG.getInitialCSS( pmapEvolvers ) );
	planarMapSVG.saveSVGFile("C:/Users/kevin/Desktop/planarmap.svg", 0);
	(*pmapEvolvers).clear(); delete pmapEvolvers;
	planarMapSVG.clearFrames();
	*/
	//pmapUpdateTime += getTime()-startTime;
	

	//Save information for svg rendering (if recording)
	//backevolvers not implemented!!
	startTime = getTime();
	if(recordingAnimation)
		svgAnimation.recordStaticFrame( &evolversThatNeedUpdates, proj, float(width()), float(height()) );
	svgUpdateTime += getTime()-startTime;
	
	//Get 3d silhouttes (as in Kalnins03)
	//evolversThatNeedUpdates[0]->get3dSilhouettes( &p_silhouette, 0.05f, 0.0f, false );

	//Print timings
	numUpdates++;
	if(numUpdates%100==0) { 
		printf("Average update:        %8.10f\n", (snaxelUpdateTime+pmapUpdateTime)/float(numUpdates));
		printf("\tSnaxel evolution:        %8.10f\n", snaxelUpdateTime/float(numUpdates));
		printf("\tPlanar map construction: %8.10f\n", pmapUpdateTime/float(numUpdates));
		printf("\tSVG construction:        %8.10f\n", svgUpdateTime/float(numUpdates));
	}
}

void GLWidget::clearSnaxels() {
	for(unsigned i = 0; i < evolvers.size(); i++)
		delete evolvers[i];
	evolvers.clear();
	evolversThatNeedUpdates.clear();

	for(unsigned i = 0; i < backevolvers.size(); i++)
		delete backevolvers[i];
	backevolvers.clear();
	backevolversThatNeedUpdates.clear();
}

void GLWidget::removeLastEvolver() {
	if( evolvers.size() > 1 ) {// Don't delete the dynamic one
		delete evolvers.back();
		evolvers.pop_back();
		evolversThatNeedUpdates.pop_back();
	}
	if( backevolvers.size() > 1 ) {// Don't delete the dynamic one
		delete backevolvers.back();
		backevolvers.pop_back();
		backevolversThatNeedUpdates.pop_back();
	}
}
