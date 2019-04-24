/*
 * common.h
 * Set of common functions/structs
 */

#ifndef COMMON_H
#define COMMON_H

#include <math.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <limits.h>
#include <float.h>
using namespace std;

#define PI 3.14159265358979323846264338327950288
#define EPSILON 10e-3

#define kalnins_intrvl 0.01f
#define kalnins_off_intrvl 0.0f
#define kalnins_sizecurve 5.f

inline float dot(float* u, float *v) { return u[0]*v[0]+u[1]*v[1]+u[2]*v[2]; }
inline float norm(float *u) { return sqrt(dot(u,u)); }
inline void normalize(float *u) { float m=norm(u); u[0]/=m; u[1]/=m; u[2]/=m; }
inline void cross(float* u, float *v, float *r) {
	r[0] = u[1]*v[2] - u[2]*v[1];
	r[1] = u[2]*v[0] - u[0]*v[2];
	r[2] = u[0]*v[1] - u[1]*v[0];
}
inline bool vec_equals(float* u, float *v) { 
	return (u[0]==v[0] && u[1]==v[1] && u[2]==v[2]);
}
inline float dist2(float *u, float *v) { 
	float w[2]={u[0]-v[0],u[1]-v[1]}; 
	return sqrt(w[0]*w[0] + w[1]*w[1]);
}
inline float dist3(float *u, float *v) { 
	float w[3]={u[0]-v[0],u[1]-v[1],u[2]-v[2]}; 
	return norm(w);
}

inline void mult4x4x1(float *A, float *v, float *r) { //A*v = r
	float norm = A[3]*v[0] + A[7]*v[1] + A[11]*v[2] + A[15]*v[3];
	r[0] = (A[0]*v[0] + A[4]*v[1] + A[ 8]*v[2] + A[12]*v[3])/norm;
	r[1] = (A[1]*v[0] + A[5]*v[1] + A[ 9]*v[2] + A[13]*v[3])/norm;
	r[2] = (A[2]*v[0] + A[6]*v[1] + A[10]*v[2] + A[14]*v[3])/norm;
}

enum NORMAL_TYPE { NO_NORMALS, HARD_NORMALS, SOFT_NORMALS };

struct Face;
struct Edge;
struct Snaxel;
class SnaxelEvolver;

struct Vertex {
	Vertex() { previouslyOccluded = occluded = false; };
	float p[3];
	float n[3];
	set<Face*> adjFaces;
	list<Vertex*> adjVerts;
	vector<Edge*> adjEdges;
	int index;
	bool visited;
	bool occluded, previouslyOccluded;
	//List of fronts (as evolvers) that currently overlap this vertex
	set<SnaxelEvolver*> frontsVisited;
	float ndotv;
	bool initialize;
};

struct Face {
	Vertex *v[3];
	float n[3];
	set<Face*> adjFaces;
	vector<Edge*> adjEdges;
	int index;
};

struct Edge {
	Edge() : ridge(false) {};
	Vertex *adjVerts[2];
	Face *adjFaces[2];
	map< SnaxelEvolver*, set<Snaxel*> > snaxels;
	float length;
	float direction[3]; //normalized direction from adjVerts[0] to adjVerts[1]
	int index;
	float n[3];
	//Ridge/valley info
	bool ridge;
	float ridge_p[3];
};

struct Snaxel {
	Snaxel(int idx) : listIdx(idx), velocity(0), acceleration(0), parent(NULL), touched(false) {};
	Snaxel(Snaxel* s) {
		p[0] = s->p[0]; p[1] = s->p[1]; p[2] = s->p[2];
		u[0] = s->u[0]; u[1] = s->u[1];
		edge = s->edge;
		flippedEdge = s->flippedEdge;
		listIdx = s->listIdx;
		left = s->left; right = s->right;
		velocity = s->velocity; acceleration = s->acceleration;
		occluded = s->occluded;
		parent = s->parent;
		touched = s->touched;
		depth = s->depth;
	};
	Edge *edge;
	bool flippedEdge;
	float p[3], u[2];
	int listIdx; //Which list the snaxel belongs to
	Snaxel *left; //Previous snaxel (counter-clockwise) 
	Snaxel *right; //Next snaxel (clockwise)
	float velocity, acceleration; //Snaxel's current velocity, acceleration (1D since edge defines direction)
	bool occluded;
	Snaxel* parent; //Parent of the snaxel from previous iteration
	bool touched; //For animation coherence
	float depth;
	int calcSize() {
		int sz=0;
		Snaxel *s = this;
		do {
			sz++;
			s = s->right;
		} while( s != this );
		return sz;
	}
};

struct SnaxelFront {
	SnaxelFront() { lastHeadUv[0] = lastHeadUv[1] = 0.f; lastHeadP[0] = 0; lastHeadP[1]=0; lastHeadP[2]=0; holeContour = false; }
	Snaxel *snaxelHead;
	int size;
	bool holeContour;
	float lastHeadUv[2], lastHeadP[3];
	float depth; //Average depth of the contour (distance from centroid to camera)
	//Sorting func
	bool operator() (SnaxelFront& f1, SnaxelFront& f2) { return f1.depth>f2.depth; };
	bool isInsideFront2d(float* u, Snaxel** nearest2dSnaxel) { 
		//Standard point-in-poly (ray tracing) technique -- shoot ray from u in direction (1,0)
		Snaxel *s = snaxelHead;
		int intersectionCount=0;
		float minDist = FLT_MAX;
		*nearest2dSnaxel = NULL;
		do {
			//Test intersection
			float d[2] = {s->right->u[0]-s->u[0], s->right->u[1]-s->u[1]};
			float t2 = (u[1]-s->u[1])/d[1]; //Intersection on snaxel edge
			float t1 = s->u[0]-u[0]+t2*d[0]; //Intersection on ray
			if( 0 <= t2 && t2 < 1 && t1 > 0 ) intersectionCount++;
			//Test 2d distance
			float dist = dist2(s->u, u);
			if(dist < minDist) {
				minDist = dist;
				*nearest2dSnaxel = s;
			}
			s = s->right;
		} while( s != snaxelHead );
		return (intersectionCount%2==1); //odd intersections => inside contour
	}
	int calcSize() { return snaxelHead->calcSize(); }
};

#endif
