/*
 * SnaxelEvolver.h
 */
 
#ifndef SNAXEL_EVOLVER_H
#define SNAXEL_EVOLVER_H

#include "common.h"
#include "Mesh.h"
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include "glew.h"
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include "timer.h"
using namespace std;

class SnaxelEvolver
{
public:
	SnaxelEvolver() {};
	SnaxelEvolver(Mesh *m, float *rgba = NULL, float minnv = 0.f, float *fixedViewpoint = NULL, bool _backfacing = false);
	~SnaxelEvolver();
	
	//OpenGL rendering of snaxels (boundaries of fronts)
	void renderSnaxels(bool dashed = false);

	//Initializations
	void initializeAroundSingleVertex(Vertex *v);
	void initializeSnaxelsOrtho(float* vv);
	void initializeSnaxelsPersp(float* vp);

	//Deforms all snaxel fronts one timestep forward
	//Returns average energy for the evolver at the last iteration
	float evolve(float timestep, float ndotv_coef, float* viewpoint, bool debug, vector<SnaxelEvolver*> *allEvolvers, bool occlusion_pass = false);

	//Same as evolve, except only moves occluded snaxels so they are visible
	void removeOcclusions(float timestep, float ndotv_coef, float* viewpoint, bool debug);
	//Sets the occluded flag for all snaxels
	bool calculateSnaxelOcclusion(); 

	//Calculate 2D projected positions of snaxels
	void projectSnaxels(float *projMatrix, float win_width, float win_height);

	//Calculate the average depth of the fronts (distance to camera), sorts them back->front
	void calculateFrontDepthAndSort(float* projMatrix);

	//Find fronts that should be rendered as "holes" in the planar map/SVG
	void calculateHoleFronts(float *projMatrix);

	//Update head snaxels so that they are near previous head snaxels (for coherent contours)
	void makeHeadsCoherent();

	//Initialize parents of the snaxels as themselves (updated during evolution to get frame-to-frame correspondences)
	void initializeParents(SnaxelEvolver* copyOfEvolver);

	//Utilities
	float *getColor() { return color; };
	void addToMinNV(float c);
	vector<SnaxelFront>* getSnaxelFronts() { return &snaxelFronts; };

	//Immitating kalnins03 (coherent stylized silhouettes)
	void get3dSilhouettes( vector< vector<Vertex> >* ps, float intrvl, float off_intrvl, bool find_pts = false );
	bool lastHeadInitialized;

private:
	//All snaxels controlled by the SnaxelEvolver
	vector<SnaxelFront> snaxelFronts;
	//Pointer to the mesh associated with these snaxels
	Mesh* mesh;
	//Minimum number of snaxels in a front before the front is removed
	int minFrontSize;
	//Largest length that a snaxel can deform for any given iteration
	float deformationThreshold;
	//Color of snaxel fronts (for visualization)
	float color[4];
	//Fixed viewpoint (for static "lights"). NULL if viewpoint is dynamic
	float *fixedVP;
	//The goal n*v -- what energy is minimized to
	float minNdotV;
	
	//Checks if snaxel front is completely contained in another front
	bool isInsideFront(const SnaxelFront& sf);

	//Deformation and fan-out, returns average energy^2 for all snaxels
	float deform(float timestep, float ndotv, float* viewpoint, vector<SnaxelEvolver*> *allEvolvers, bool occlusion_pass = false);
	//Cleaning conquest
	void clean();
	//Split snaxel multiple snaxels at self-intersection points
	bool split();
	//Merge intersecting snaxels
	bool merge();

	bool backfacing; // Backfacing evolver
};

#endif
