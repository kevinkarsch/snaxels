/*
 * SVGCreator.h
 */
 
#ifndef SVGCREATOR_H
#define SVGCREATOR_H

#include "common.h"
#include "SnaxelEvolver.h"
#include <QStringList>
#include <iostream>
#include <fstream>
using namespace std;

struct SVGContour {
	SVGContour(QString _id) : id(_id) {};
	QString id;
	vector<bool> hole;
	vector< pair<float,float> > centroid;
	vector< vector< pair<float,float> > > fill;
	vector< vector< pair<float,float> > > stroke;
	vector< vector< pair<float,float> > > marker;
};

class SVGCreator
{
public:
	SVGCreator(int w=0, int h=0);
	~SVGCreator();
	

	void recordStaticFrame( vector<SnaxelEvolver*>* evolvers, float *proj, float w, float h  );
	void clearFrames() { frames.clear(); };
	int nFrames() { return frames.size(); };

	void setDefs(QString& defsbody);
	void setCSS(QString& cssbody);
	QString getInitialCSS(vector<SnaxelEvolver*>* evolvers);

	QString svgFileAsString(int frameN);
	void saveSVGFile(const QString& fileName, int frameN);

	void recordDynamicFrame( vector<SnaxelEvolver*>* evolvers, Mesh* mesh );
	void eraseDynamicFrames();
	void prepareAnimationCoherence();
	void saveSVGAnimation(const QString& fileName);
	void saveSVGAnimationFB(const QString& fileName);

private:
	int width, height;
	vector< vector<SVGContour> > frames;
	QString svgHeader, svgFooter, svgDefs, svgCSS;
	QString svgBody(int frameN, bool noFill = false);
	QString svgPath( vector< pair<float,float> >& p, pair<float,float> *centroid, QString cid, QString maskid=QString("") );
	QString svgMasksFromContour( SVGContour& c, QString maskid );

	//Keeps a copy of snaxel fronts per clip, and per frame, per evolver (for animation)
	vector< vector< vector<SnaxelEvolver*> > > dEvolvers;
};

#endif
