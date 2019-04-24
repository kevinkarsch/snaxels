/*
 * Mesh.h
 * Mesh class for loading OFF/PLY mesh files
 * Holds a vector containing all active snaxels
 */
 
#ifndef MESH_H
#define MESH_H

#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <float.h>
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
#include "rply.h"
#include "timer.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

using namespace std;

class Mesh
{
public:
    Mesh();
	~Mesh();
	void loadFromFile(string file);
	void centerAndNormalizeMesh();
	inline vector<Face*> & getFaces() { return faces; };
	inline vector<Vertex*> & getVertices() { return verts; };
	inline vector<Edge*> & getEdges() { return edges; };
	void render(GLenum face, GLenum mode, NORMAL_TYPE nt, bool renderFronts, bool forSelection = false);
	void clearMesh();
	void calculateNormals();
	void findAdjacencies();
	void calculateOccludedVerts();
	void translate(float tx, float ty, float tz);
	void rotate(float rx, float ry, float rz);
	void scale(float sx, float sy, float sz);
	inline float* getTform() { return &tform[0]; };

private:
	vector<Face*> faces;
	vector<Vertex*> verts;
	vector<Edge*> edges;

	GLfloat tform[16];
	
	void makeIdentityTform();
	int loadPLY(string file);
	int loadOFF(string file);
	void loadOBJ(string file);
};

#endif
