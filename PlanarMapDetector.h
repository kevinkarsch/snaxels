/*
 * PlanarMapDetector.h
 */
 
#ifndef PLANAR_MAP_DETECTOR_H
#define PLANAR_MAP_DETECTOR_H

#include "common.h"
#include "SnaxelEvolver.h"
#include "gpc.h"
using namespace std;

class PlanarMapDetector
{
public:
	PlanarMapDetector();
	~PlanarMapDetector();

	//Finds a planar map for the evolver wrt the current viewpoint's proj. matrix (P)
	vector<gpc_polygon>* findPlanarMap(vector<SnaxelEvolver*>& evols, float *P, vector<SnaxelEvolver*>** evolsOut = NULL, int w=0, int h=0);

	//Renders planar map to screen space
	void renderPlanarMap( vector<gpc_polygon>* p );
	
	//Frees memory associated with planar map polygons
	void freeMap( vector<gpc_polygon>* p ) {
		if(p == NULL) return;
		for(unsigned i = 0; i < p->size(); i++) gpc_free_polygon( &((*p)[i]) );
		delete p;
	}

private:

};

#endif
