#include "Mesh.h"
#include "SnaxelEvolver.h"

Mesh::Mesh() {
	makeIdentityTform();
}

Mesh::~Mesh() {
	clearMesh();
}

unsigned xi, yi, zi, fi;
int ply_vertex_cb(p_ply_argument argument) {
	p_ply_property prop;
	long plen, pind;
	ply_get_argument_property(argument, &prop, &plen, &pind);
	const char *pname;
	ply_get_property_info(prop, &pname, NULL, NULL, NULL);
	void *data;
	ply_get_argument_user_data(argument, &data, NULL);
	Mesh* m = (Mesh*)data;
	if(xi == yi && yi == zi) m->getVertices().push_back(new Vertex());
	if( strcmp(pname, "x")==0 ) {
		m->getVertices()[ xi++ ]->p[0] = ply_get_argument_value(argument);
	} else if( strcmp(pname, "y")==0 ) {
		m->getVertices()[ yi++ ]->p[1] = ply_get_argument_value(argument);
	} else if( strcmp(pname, "z")==0 ) {
		m->getVertices()[ zi++ ]->p[2] = ply_get_argument_value(argument);
	}
    return 1;
}

int ply_face_cb(p_ply_argument argument) {
	void *data;
	ply_get_argument_user_data(argument, &data, NULL);
	Mesh* m = (Mesh*)data;
    long length, f_index;
    ply_get_argument_property(argument, NULL, &length, &f_index);
	int v_index = ply_get_argument_value(argument);
	if( f_index < 0 ) {
		m->getFaces().push_back( new Face() );
		fi++;
	} else if( f_index < 3 ) {
		m->getFaces()[ fi-1 ]->v[ f_index ] = m->getVertices()[v_index];
		m->getVertices()[ v_index ]->adjFaces.insert(m->getFaces()[fi-1]);
	}
    return 1;
}

int Mesh::loadPLY(string file) {
	//Open the file
    p_ply ply = ply_open(file.c_str(), NULL);
    if (!ply) return 1;
    if (!ply_read_header(ply)) return 1;
	//Initialize global counters
	xi = yi = zi = fi = 0;
	//Set the callbacks
	long numV, numF;
    numV = ply_set_read_cb(ply, "vertex", "x", ply_vertex_cb, (void*)this, 0);
    ply_set_read_cb(ply, "vertex", "y", ply_vertex_cb, (void*)this, 0);
    ply_set_read_cb(ply, "vertex", "z", ply_vertex_cb, (void*)this, 0);
	numF = ply_set_read_cb(ply, "face", "vertex_indices", ply_face_cb, 
		(void*)this, 0);
	printf("\t# Vertices: %ld\n\t# Faces: %ld\n", numV, numF);
	clearMesh();
	//Execute callbacks
    if (!ply_read(ply)) return 1;
    ply_close(ply);
    return 0;
}

int Mesh::loadOFF(string file) {
	ifstream filestream(file.c_str());
	string header; 
	filestream >> header; //"OFF" string, hopefully
	if( header.compare("OFF") != 0 ) {
		printf("Error: invalid OFF file - first line of file is not \"OFF\"\n");
		return 1;
	}
	clearMesh();
	long numV, numF, numE; 
	filestream >> numV; //numVerts
	filestream >> numF; //numFaces
	filestream >> numE; //numEdges
	printf("\t# Vertices: %ld\n\t# Faces: %ld\n", numV, numF);
	for(long i = 0; i < numV; i++) {
		verts.push_back(new Vertex());
		filestream >> verts[i]->p[0];
		filestream >> verts[i]->p[1];
		filestream >> verts[i]->p[2];
		verts[i]->index = i;
	}
	int numVertsInFace; //unused
	int v0, v1, v2;
	for(long i = 0; i < numF; i++) {
		faces.push_back(new Face());
		filestream >> numVertsInFace;
		filestream >> v0; faces[i]->v[0] = verts[v0];
		filestream >> v1; faces[i]->v[1] = verts[v1];
		filestream >> v2; faces[i]->v[2] = verts[v2];
		//Set the faces to the vertices
		verts[v0]->adjFaces.insert( faces[i] );
		verts[v1]->adjFaces.insert( faces[i] );
		verts[v2]->adjFaces.insert( faces[i] );
		faces[i]->index = i;
	}
	filestream.close();
	return 0;
}

void Mesh::loadOBJ(string file) {

}

void Mesh::loadFromFile(string file) {
	double startTime = getTime();
	cout << file << endl;
	printf("Reading file: %s\n", file.c_str());
	//Find the file extension
	string::size_type idx;
	idx = file.rfind(".");
	string ext; ext.clear();
	if(idx != string::npos)
		ext = file.substr(idx+1);
	if( ext.empty() ) {
		printf("Error: unknown file type (no extension)\n");
		return;
	}
	//Convert extension to lower case
	string::iterator it;
	for(it = ext.begin(); it != ext.end(); it++) *it = tolower( *it );

	if( ext.compare("ply")==0 )
		loadPLY( file );
	else if( ext.compare("off")==0 )
		loadOFF( file );
	else if( ext.compare("obj")==0 )
		loadOBJ( file );
	else
		printf("Error: unknown extension .%s\n", ext.c_str());
	centerAndNormalizeMesh();
	printf("Done (%.4fs).\n\n", getTime()-startTime);
}

void Mesh::centerAndNormalizeMesh() {
	//Find bounds, ensure modelview matrix places mesh in a [-0.5,0.5]^3
	float minv[3] = {FLT_MAX,FLT_MAX,FLT_MAX};
	float maxv[3] = {-FLT_MAX,-FLT_MAX,-FLT_MAX};
	for(unsigned i = 0; i < verts.size(); i++) {
		if	   ( verts[i]->p[0] < minv[0] ) minv[0] = verts[i]->p[0];
		else if( verts[i]->p[0] > maxv[0] ) maxv[0] = verts[i]->p[0];
		if	   ( verts[i]->p[1] < minv[1] ) minv[1] = verts[i]->p[1];
		else if( verts[i]->p[1] > maxv[1] ) maxv[1] = verts[i]->p[1];
		if	   ( verts[i]->p[2] < minv[2] ) minv[2] = verts[i]->p[2];
		else if( verts[i]->p[2] > maxv[2] ) maxv[2] = verts[i]->p[2];
	}
	float s = 1.f/max( max(maxv[0]-minv[0], maxv[1]-minv[1]), maxv[2]-minv[2] );
	float t[3] = {0.5*(maxv[0]+minv[0]), 0.5*(maxv[1]+minv[1]), 0.5*(maxv[2]+minv[2])};
	for(unsigned i = 0; i < verts.size(); i++) {
		verts[i]->p[0] = (verts[i]->p[0]-t[0])*s;
		verts[i]->p[1] = (verts[i]->p[1]-t[1])*s;
		verts[i]->p[2] = (verts[i]->p[2]-t[2])*s;
	}
}

float *getFrontColor(Vertex* v, float* c) {
	if( v->frontsVisited.empty() ) {
		c[0] = c[1] = c[2] = 0.8f; c[3] = 1.f;
	} else {
		c[0] = c[1] = c[2] = c[3] = 0.f;
		for(set<SnaxelEvolver*>::iterator it = v->frontsVisited.begin();
										  it != v->frontsVisited.end(); it++) {
			float *tmpc = (*it)->getColor();
			c[0] += tmpc[0]; c[1] += tmpc[1]; c[2] += tmpc[2];
			c[3] = max(c[3], tmpc[3]);
		}
		c[0] = min(1.f, c[0]);
		c[1] = min(1.f, c[1]);
		c[2] = min(1.f, c[2]);
	}
	return c;
}

void Mesh::render(GLenum face, GLenum mode, NORMAL_TYPE nt, bool renderFronts, bool forSelection) {
	glPushMatrix();
	glMultMatrixf( tform );
	vector<Face*>::iterator it;
	glPolygonMode(face, mode);
	if(!forSelection) {
		if(nt == NO_NORMALS) glDisable(GL_LIGHTING);
		else glEnable(GL_LIGHTING);
		float c[4];
		glBegin( GL_TRIANGLES ); {
			switch( nt ) {
				case NO_NORMALS:
					for( it = faces.begin(); it != faces.end(); it++ ) {
						if(renderFronts) glColor4fv( getFrontColor((*it)->v[0], c) );
						glVertex3fv( (*it)->v[0]->p );
						if(renderFronts) glColor4fv( getFrontColor((*it)->v[1], c) );
						glVertex3fv( (*it)->v[1]->p );
						if(renderFronts) glColor4fv( getFrontColor((*it)->v[2], c) );
						glVertex3fv( (*it)->v[2]->p );
					}
				break;
				case HARD_NORMALS:
					for( it = faces.begin(); it != faces.end(); it++ ) {
						glNormal3fv( (*it)->n );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[0], c));
						glVertex3fv( (*it)->v[0]->p );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[1], c));
						glVertex3fv( (*it)->v[1]->p );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[2], c));
						glVertex3fv( (*it)->v[2]->p );
					}
				break;
				case SOFT_NORMALS:
					for( it = faces.begin(); it != faces.end(); it++ ) {
						glNormal3fv( (*it)->v[0]->n );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[0], c));
						glVertex3fv( (*it)->v[0]->p );
						glNormal3fv( (*it)->v[1]->n );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[1], c));
						glVertex3fv( (*it)->v[1]->p );
						glNormal3fv( (*it)->v[2]->n );
						if(renderFronts) glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, getFrontColor((*it)->v[2], c));
						glVertex3fv( (*it)->v[2]->p );
					}
				break;
			}
		} glEnd();
	} else {
		glDisable(GL_LIGHTING);
		//Draw mesh into depth channel
		glColorMask(false,false,false,false);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0, 2.0);
		glBegin(GL_TRIANGLES);
		for( it = faces.begin(); it != faces.end(); it++ ) {
			glVertex3fv( (*it)->v[0]->p );
			glVertex3fv( (*it)->v[1]->p );
			glVertex3fv( (*it)->v[2]->p );
		}
		glEnd();
		glDisable(GL_POLYGON_OFFSET_FILL);
		glColorMask(true,true,true,true);
		//Render selection points
		glPointSize(10.f);
		glBegin( GL_POINTS );
		for(unsigned i = 0; i < verts.size(); i++) {
			glColor3ub(int(floor(float(i)/65536.f)), int(floor(float(i)/256.f)), i%256);
			glNormal3fv( verts[i]->n );
			glVertex3fv( verts[i]->p );
		}
		glEnd();
		glPointSize(1.f);
	}
	glPopMatrix();
	//calculateOccludedVerts();
}

void Mesh::calculateNormals() {
	double startTime = getTime();
	printf("Calculating face and vertex normals.\n");
	for(unsigned i = 0; i < verts.size(); i++) {
		verts[i]->n[0] = 0; verts[i]->n[1] = 0; verts[i]->n[2] = 0;
	}
	float v1[3], v2[3];
	for(unsigned i = 0; i < faces.size(); i++) {
		v1[0] = faces[i]->v[0]->p[0] - faces[i]->v[1]->p[0];
		v1[1] = faces[i]->v[0]->p[1] - faces[i]->v[1]->p[1];
		v1[2] = faces[i]->v[0]->p[2] - faces[i]->v[1]->p[2];
		v2[0] = faces[i]->v[0]->p[0] - faces[i]->v[2]->p[0];
		v2[1] = faces[i]->v[0]->p[1] - faces[i]->v[2]->p[1];
		v2[2] = faces[i]->v[0]->p[2] - faces[i]->v[2]->p[2];
		cross(v1, v2, faces[i]->n);
		float faceArea_x_2 = norm(faces[i]->n); //Divide by 2 for face area
		normalize( faces[i]->n );
		faces[i]->v[0]->n[0] += faceArea_x_2 * faces[i]->n[0];
		faces[i]->v[0]->n[1] += faceArea_x_2 * faces[i]->n[1];
		faces[i]->v[0]->n[2] += faceArea_x_2 * faces[i]->n[2];
		faces[i]->v[1]->n[0] += faceArea_x_2 * faces[i]->n[0];
		faces[i]->v[1]->n[1] += faceArea_x_2 * faces[i]->n[1];
		faces[i]->v[1]->n[2] += faceArea_x_2 * faces[i]->n[2];
		faces[i]->v[2]->n[0] += faceArea_x_2 * faces[i]->n[0];
		faces[i]->v[2]->n[1] += faceArea_x_2 * faces[i]->n[1];
		faces[i]->v[2]->n[2] += faceArea_x_2 * faces[i]->n[2];
	}
	for(unsigned i = 0; i < verts.size(); i++) {
		normalize( verts[i]->n );
	}
	printf("Done (%.4fs).\n\n", getTime()-startTime);
}

void Mesh::calculateOccludedVerts() {
	glPushMatrix();
	glMultMatrixf( tform );
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
	GLuint sampleCount, query;
	glGenQueriesARB(1, &query);
	for(unsigned i = 0; i < verts.size(); i++ ) {
		glBeginQueryARB(GL_SAMPLES_PASSED_ARB, query);
		glBegin(GL_POINTS);
		glVertex3fv( verts[i]->p );
		glEnd();
		glEndQueryARB(GL_SAMPLES_PASSED_ARB);
		glGetQueryObjectuivARB(query, GL_QUERY_RESULT_ARB, &sampleCount);
		verts[i]->previouslyOccluded = verts[i]->occluded;
		verts[i]->occluded = (sampleCount>0 ? false : true);
	}
	glDeleteQueriesARB(1, &query);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
	glPopMatrix();
}

#include <iterator>
void Mesh::findAdjacencies() {
	double startTime = getTime();
	printf("Finding adjacencies for all vertices and faces.\n");
	set<Face*>::iterator s_it;
	//Add all adjacent vertices to each vertex, make sure ordering is consistent
	for(unsigned i = 0; i < verts.size(); i++) {
		set<Vertex*> insertedSet;
		Face *init_f = *(verts[i]->adjFaces.begin());
		if( init_f->v[0] == verts[i] ) { 
			verts[i]->adjVerts.push_back( init_f->v[1] );
			verts[i]->adjVerts.push_back( init_f->v[2] );
		} else if( init_f->v[1] == verts[i] ) { 
			verts[i]->adjVerts.push_back( init_f->v[2] );
			verts[i]->adjVerts.push_back( init_f->v[0] );
		}  else {
			verts[i]->adjVerts.push_back( init_f->v[0] );
			verts[i]->adjVerts.push_back( init_f->v[1] );
		}
		insertedSet.insert( init_f->v[0] );
		insertedSet.insert( init_f->v[1] );
		insertedSet.insert( init_f->v[2] );

		s_it = verts[i]->adjFaces.begin(); s_it++;
		for( ; s_it != verts[i]->adjFaces.end(); s_it++) {
			Vertex *lastVert = verts[i]->adjVerts.back();
			set<Face*> edgeFaces;
			set_intersection( verts[i]->adjFaces.begin(), verts[i]->adjFaces.end(), 
							  lastVert->adjFaces.begin(), lastVert->adjFaces.end(), 
							  inserter(edgeFaces,edgeFaces.begin()) );
			pair<set<Vertex*>::iterator,bool> ret;
			int count = 0;
			do {
				if(count < 3)
					ret = insertedSet.insert( (*(edgeFaces.begin()))->v[ count ] );
				else
					ret = insertedSet.insert( (*(--edgeFaces.end()))->v[ count-3 ] );
			} while( (! ret.second) && (++count <= 5) );
			if( ret.second )
				verts[i]->adjVerts.push_back( *ret.first );
		}
	}
	//Add all adjacent faces to each face
	vector<Face*>::iterator it;
	set<Face*> tmp1, tmp2, tmp3;
	for( it = faces.begin(); it != faces.end(); it++ ) {
		tmp1.clear();
		set_intersection( (*it)->v[0]->adjFaces.begin(), 
			(*it)->v[0]->adjFaces.end(), (*it)->v[1]->adjFaces.begin(),
			(*it)->v[1]->adjFaces.end(), inserter(tmp1,tmp1.begin()) );
		tmp2.clear();
		set_intersection( (*it)->v[0]->adjFaces.begin(), 
			(*it)->v[0]->adjFaces.end(), (*it)->v[2]->adjFaces.begin(),
			(*it)->v[2]->adjFaces.end(), inserter(tmp2,tmp2.begin()) );
		tmp3.clear();
		set_intersection( (*it)->v[2]->adjFaces.begin(), 
			(*it)->v[2]->adjFaces.end(), (*it)->v[1]->adjFaces.begin(),
			(*it)->v[1]->adjFaces.end(), inserter(tmp3,tmp3.begin()) );
		(*it)->adjFaces.clear();
		(*it)->adjFaces.insert( tmp1.begin(), tmp1.end() );
		(*it)->adjFaces.insert( tmp2.begin(), tmp2.end() );
		(*it)->adjFaces.insert( tmp3.begin(), tmp3.end() );
		(*it)->adjFaces.erase( (*it) );
	}
	//Create edges
	for(unsigned i = 0; i < verts.size(); i++) 
		verts[i]->visited = false;
	for(unsigned i = 0; i < verts.size(); i++) {
		verts[i]->visited = true;
		for(list<Vertex*>::iterator lv_it = verts[i]->adjVerts.begin(); 
									lv_it != verts[i]->adjVerts.end(); lv_it++) {
			if((*lv_it)->visited) continue;
			//Get edge details
			edges.push_back(new Edge);
			edges.back()->adjVerts[0] = verts[i];
			edges.back()->adjVerts[1] = *lv_it;
			set<Face*> edgeFaces;
			set_intersection( verts[i]->adjFaces.begin(), verts[i]->adjFaces.end(), 
							  (*lv_it)->adjFaces.begin(), (*lv_it)->adjFaces.end(), 
							  inserter(edgeFaces,edgeFaces.begin()) );
			set<Face*>::iterator face1 = edgeFaces.begin();
			edges.back()->adjFaces[0] = *face1;
			edges.back()->adjFaces[1] = *(++face1);
			//Other stats
			edges.back()->direction[0] = edges.back()->adjVerts[1]->p[0] - edges.back()->adjVerts[0]->p[0];
			edges.back()->direction[1] = edges.back()->adjVerts[1]->p[1] - edges.back()->adjVerts[0]->p[1];
			edges.back()->direction[2] = edges.back()->adjVerts[1]->p[2] - edges.back()->adjVerts[0]->p[2];
			edges.back()->length = norm( edges.back()->direction );
			edges.back()->direction[0] /= edges.back()->length;
			edges.back()->direction[1] /= edges.back()->length;
			edges.back()->direction[2] /= edges.back()->length;
			edges.back()->index = edges.size()-1;
			edges.back()->n[0] = 0.5f*(edges.back()->adjFaces[0]->n[0] + edges.back()->adjFaces[1]->n[0]);
			edges.back()->n[1] = 0.5f*(edges.back()->adjFaces[0]->n[1] + edges.back()->adjFaces[1]->n[1]);
			edges.back()->n[2] = 0.5f*(edges.back()->adjFaces[0]->n[2] + edges.back()->adjFaces[1]->n[2]);
			//Store edges in other mesh data types
			edges.back()->adjVerts[0]->adjEdges.push_back( edges.back() );
			edges.back()->adjVerts[1]->adjEdges.push_back( edges.back() );
			edges.back()->adjFaces[0]->adjEdges.push_back( edges.back() );
			edges.back()->adjFaces[1]->adjEdges.push_back( edges.back() );
		}
	}
	for(unsigned i = 0; i < verts.size(); i++) 
		verts[i]->visited = false;
	//Wind the edges properly
	for(unsigned i = 0; i < verts.size(); i++) {
		vector<Edge*> newAdjEdges;
		for(list<Vertex*>::iterator lv_it = verts[i]->adjVerts.begin(); 
									lv_it != verts[i]->adjVerts.end(); lv_it++) {
			for(unsigned j = 0; j < verts[i]->adjEdges.size(); j++) {
				if( (verts[i]->adjEdges[j]->adjVerts[0] == verts[i] && verts[i]->adjEdges[j]->adjVerts[1] == (*lv_it)) ||
					(verts[i]->adjEdges[j]->adjVerts[1] == verts[i] && verts[i]->adjEdges[j]->adjVerts[0] == (*lv_it)) )
				{
					newAdjEdges.push_back( verts[i]->adjEdges[j] );
					break;
				}
			}
		}
		copy( newAdjEdges.begin(), newAdjEdges.end(), verts[i]->adjEdges.begin() );
	}
	printf("Done (%.4fs).\n\n", getTime()-startTime);
}

void Mesh::translate(float tx, float ty, float tz) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf( tform );
	glTranslatef(tx, ty, tz);
	glGetFloatv(GL_MODELVIEW_MATRIX, tform);
	glPopMatrix();
}

void Mesh::rotate(float rx, float ry, float rz) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf( tform );
	glRotatef(rx, 1, 0, 0);
	glRotatef(ry, 0, 1, 0);
	glRotatef(rz, 0, 0, 1);
	glGetFloatv(GL_MODELVIEW_MATRIX, tform);
	glPopMatrix();
}

void Mesh::scale(float sx, float sy, float sz) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf( tform );
	glScalef(sx, sy, sz);
	glGetFloatv(GL_MODELVIEW_MATRIX, tform);
	glPopMatrix();
}

void Mesh::makeIdentityTform() {
	tform[0] = tform[5] = tform[10] = tform[15] = 1;
	tform[1] = tform[2] = tform[3] = tform[4] = 0;
	tform[6] = tform[7] = tform[8] = tform[9] = 0;
	tform[11] = tform[12] = tform[13] = tform[14] = 0;
}

void Mesh::clearMesh() {
	makeIdentityTform();
	for(unsigned i = 0; i < verts.size(); i++)
		delete verts[i];
	verts.clear();
	for(unsigned i = 0; i < faces.size(); i++)
		delete faces[i];
	faces.clear();
	for(unsigned i = 0; i < edges.size(); i++)
		delete edges[i];
	edges.clear();
}
