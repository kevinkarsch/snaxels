#include <QtGui>

#include "GLWindow.h"
#include "SnaxelCreatorWidget.h"

GLWindow::GLWindow( QWidget *parent ) {
    setupUi(this); // this sets up GUI

	QObject::connect(actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
	QObject::connect(actionAdd_front, SIGNAL(triggered()), this, 
					 SLOT(addFront()));
	QObject::connect(actionRemove_last_front, SIGNAL(triggered()), this, 
					 SLOT(removeLastFront()));
	QObject::connect(actionToggle_deformation, SIGNAL(triggered()), this, 
					 SLOT(toggleTimer()));
	QObject::connect(actionRender_fronts, SIGNAL(triggered()), this, 
					 SLOT(toggleRenderFronts()));
	QObject::connect(actionIncrease_goal_N_V, SIGNAL(triggered()), this, 
					 SLOT(increaseGoalNV()));
	QObject::connect(actionDecrease_goal_N_V, SIGNAL(triggered()), this, 
					 SLOT(decreaseGoalNV()));
	QObject::connect(actionExport_all_pose_projections, SIGNAL(triggered()), this, 
					 SLOT(exportPoseProjections()));
	QObject::connect(recordButton, SIGNAL(clicked()), this, 
					 SLOT(toggleRecording()));
	QObject::connect(frameSlider, SIGNAL(valueChanged(int)), this, 
					 SLOT(updateSVG()));
	QObject::connect(defsText, SIGNAL(textChanged()), this, 
					 SLOT(updateSVG()));
	QObject::connect(cssText, SIGNAL(textChanged()), this, 
					 SLOT(updateSVG()));
	QObject::connect(depthList, SIGNAL(indexesMoved(const QModelIndexList&)), this, 
					 SLOT(updateSVG()));
	QObject::connect(actionExport_animation, SIGNAL(triggered()), this, 
					 SLOT(saveAnimation()));
	QObject::connect(actionSave_GL_frame, SIGNAL(triggered()), this, 
					 SLOT(saveGLFrame()));
	QObject::connect(actionSave_SVG_frame, SIGNAL(triggered()), this, 
					 SLOT(saveSVGFrame()));

	QObject::connect(actionRidge_mode, SIGNAL(triggered()), this, 
					 SLOT(toggleRidgeMode()));

	glWidget->svgWidget = svgWidget;
	svgWidget->setAnimation(&(glWidget->svgAnimation));
}

void GLWindow::openFile() {
	QString fileName =
		QFileDialog::getOpenFileName(this, tr("Open Mesh File"), 
			"./", tr("Mesh Files (*.off *.ply)")); //Obj loader not yet implemented
	if( ! fileName.isEmpty() ) {
		glWidget->clearSnaxels();
		glWidget->getMesh()->loadFromFile( std::string(fileName.toAscii().constData(), fileName.toAscii().length()) );
		glWidget->getMesh()->calculateNormals();
		glWidget->getMesh()->findAdjacencies();
		float rgba[4] = {0,1,0,1};
		glWidget->createEvolver(rgba, 0.0f, NULL);
		glWidget->updateGL();
	}
	updateSVG();
}

void GLWindow::addFront() {
	if( glWidget->getEvolvers()->empty() ) 
		return;
	//Get snaxel front color
	qsrand(getTime());
	QColor initialColor; initialColor.setHsv( qrand()%255, 255, 255 );
	QColor frontColor = QColorDialog::getColor( initialColor, NULL, 
		"Snaxel front color", QColorDialog::ShowAlphaChannel );
	if( ! frontColor.isValid() ) //User clicked cancel
		return;
	//Get current viewpoint/selected vertex normal
	float vp[3]; 
	if(glWidget->selectedVertex==NULL) {
		glWidget->getViewpoint(vp);
	} else { //Get vertex vp (very far distance away in direction of normal)
		Vertex *sv = glWidget->selectedVertex;
		vp[0] = sv->p[0] + 10000.f*sv->n[0];
		vp[1] = sv->p[1] + 10000.f*sv->n[1];
		vp[2] = sv->p[2] + 10000.f*sv->n[2];
	}
	//Get additional front parameters
	SnaxelCreatorWidget *scw = new SnaxelCreatorWidget(this);
	scw->exec(); //Wait for user provide input
	float nvmin=0;
	scw->getParameters(&nvmin);
	delete scw;
	//Create the front
	float rgba[4] = {frontColor.redF(), frontColor.greenF(),
					 frontColor.blueF(), frontColor.alphaF()};
	//Add a new evolver?
//vp[0] = 10000.f*1.f; vp[1] = 10000.f*1.f; vp[2] = 10000.f*1.f; // Static light source
glWidget->createEvolver(rgba, nvmin, vp, glWidget->selectedVertex);
	//Or add a new front?
//glWidget->getEvolvers()->back()->initializeAroundSingleVertex( glWidget->selectedVertex );
	glWidget->updateGL();
}

void GLWindow::removeLastFront() {
	glWidget->removeLastEvolver();
	glWidget->updateGL();
}

void GLWindow::toggleTimer() {
	glWidget->toggleDeformationTimer();
}

void GLWindow::toggleRenderFronts() {
	glWidget->toggleRenderFronts();
	glWidget->updateGL();
}

void GLWindow::increaseGoalNV() {
	if( glWidget->getEvolvers()->empty() ) return;
	glWidget->getEvolvers()->back()->addToMinNV( 0.01 );
	glWidget->deformSnaxels();
}

void GLWindow::decreaseGoalNV() {
	if( glWidget->getEvolvers()->empty() ) return;
	glWidget->getEvolvers()->back()->addToMinNV( -0.01 );
	glWidget->deformSnaxels();
}

void GLWindow::exportPoseProjections() {
	float THETA_INC=1.f, PHI_INC=1.f; //Degrees in between each saved view
	QString dirName =
		QFileDialog::getExistingDirectory(this, tr("Choose directory to save poses to"), 
			"./",  QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
	if( ! dirName.isEmpty() ) {
		int theta_itr = (int)floor(360.f/THETA_INC);
		float theta_real = 360.f / float(theta_itr);
		int phi_itr = (int)floor(180.f/PHI_INC)+1;
		float phi_real = 180.f / float(phi_itr);
		for(int i = 0; i <= phi_itr; i++) {
			QString dirNameItr = dirName+QString("/%1").arg((int)i,3,10,QLatin1Char('0'));
			QDir dirclass;
			dirclass.mkdir(dirNameItr);
			float phi = float(i-phi_itr/2)*phi_real * PI / 180.f;
			for(int j = 0; j < theta_itr; j++) {
				float theta = float(j)*theta_real * PI / 180.f;
				glWidget->rotateView(theta, phi);
				glWidget->initializeSnaxels();
				glWidget->deformSnaxels();
				glWidget->updateGL();
				//glWidget->grabFrameBuffer().save(dirNameItr+QString("/%1.png").arg((int)j,3,10, QLatin1Char('0')));
			}
		}
	}
}

void GLWindow::toggleRecording() {
	QString idleText = "Record\nAnimation";
	QString recordingText = "Recording\n(Press to end)";
	if(recordButton->text() == idleText) { //Idle -> Recording
		recordButton->setText(recordingText);
		glWidget->svgAnimation.clearFrames();
		glWidget->svgAnimation.eraseDynamicFrames();
		glWidget->recordingAnimation = true;
	} else if(recordButton->text() == recordingText) { //Recording -> Idle
		recordButton->setText(idleText);
		frameSlider->setRange(0, glWidget->svgAnimation.nFrames()-1);
		frameSlider->setValue(0);
		cssText->setPlainText( svgWidget->getInitialCSS(glWidget->getEvolvers()) );
		updateSVG();
		glWidget->recordingAnimation = false;
		glWidget->svgAnimation.prepareAnimationCoherence();
	}
}

void GLWindow::updateSVG() {
	//Implement depth ordering here
	QString defsbody = defsText->toPlainText();
	svgWidget->setDefs(defsbody);
	glWidget->planarMapSVG.setCSS(defsbody);
	QString cssbody = cssText->toPlainText();
	svgWidget->setCSS(cssbody);
	svgWidget->setFrame(frameSlider->value());
}

void GLWindow::saveAnimation() {
	QString fileName =
		QFileDialog::getSaveFileName(this, tr("Save animation"), 
			"./", tr("Animation formats (*.svg *.swf)"));
	if( ! fileName.isEmpty() ) {
		if(fileName.endsWith(".svg"))
			svgWidget->saveSVGAnimation(fileName);
		else if(fileName.endsWith(".swf"))
			svgWidget->saveSWFAnimation(fileName);
		else
			svgWidget->saveSVGAnimation(fileName + QString(".svg"));
	}
}

void GLWindow::saveGLFrame() {
	QString fileName =
		QFileDialog::getSaveFileName(this, tr("Save GL Frame"), 
			"./", tr("PNG image (*.png)"));
	if( ! fileName.isEmpty() ) {
		if(fileName.endsWith(".png"))
			glWidget->grabFrameBuffer().save(fileName);
		else
			glWidget->grabFrameBuffer().save(fileName+QString(".png"));
	}
}

void GLWindow::saveSVGFrame() {
	QString fileName =
		QFileDialog::getSaveFileName(this, tr("Save SVG Frame"), 
			"./", tr("SVG image (*.svg)"));
	if( ! fileName.isEmpty() ) {
		int frameNumber = frameSlider->value();
		if(fileName.endsWith(".svg"))
			svgWidget->getAnimation()->saveSVGFile(fileName, frameNumber);
		else
			svgWidget->getAnimation()->saveSVGFile(fileName+QString(".svg"), frameNumber);
	}
}

void GLWindow::toggleRidgeMode() {
	if( !glWidget->ridgeModeOn ) {
		//Clear ridges
		for(unsigned i = 0; i < glWidget->getMesh()->getEdges().size(); i++)
			glWidget->getMesh()->getEdges()[i]->ridge = false;
	}
	glWidget->ridgeModeOn = !glWidget->ridgeModeOn;
}