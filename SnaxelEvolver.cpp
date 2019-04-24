#include "SnaxelEvolver.h"
#include <time.h>

SnaxelEvolver::SnaxelEvolver(Mesh *m, float *rgba, float minnv, 
		float *fixedViewpoint, bool _backfacing) {
	if(m) {
		mesh = m;
		//Find minimum front size (= minimum # of valence verts for a given vertex)
		//TODO: this is not currently used, assumed to be 4
		minFrontSize = INT_MAX;
		vector<Vertex*> verts = mesh->getVertices();
		for(unsigned i = 0; i < verts.size(); i++) {
			if( minFrontSize > int(verts[i]->adjVerts.size()) )
				minFrontSize = int(verts[i]->adjVerts.size());
		}
		if( minFrontSize < 4 ) minFrontSize = 4;
		minFrontSize = 4; 
		//Find largest deformation per iteration (=smallest edge length in mesh)
		// This is only applied when a snaxel lies on a visual contour edge
		deformationThreshold = FLT_MAX;
		vector<Edge*> edges = mesh->getEdges();
		for(unsigned i = 0; i < edges.size(); i++) {
			if( deformationThreshold > edges[i]->length )
				deformationThreshold = edges[i]->length / 1.1f; 
		}
	}
	//Copy color
	if(rgba==NULL) { 
		color[0] = 1; color[1] = .3; color[2] = .2; color[3] = 1; 
	} else {
		memcpy(color, rgba, sizeof(float)*4);
	}
	//Copy vp (if exists)
	if( fixedViewpoint ) {
		fixedVP = new float[3]; 
		memcpy(fixedVP, fixedViewpoint, sizeof(float)*3);
	} else {
		fixedVP = NULL;
	}
	//Copy minimum n*v
	minNdotV = minnv;
	backfacing = _backfacing;

	lastHeadInitialized = false;
}

SnaxelEvolver::~SnaxelEvolver() {
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		Snaxel *stopSnaxel = snaxelFronts[i].snaxelHead->left;
		while( s != stopSnaxel ) {
			Snaxel *s_right = s->right;
			s->edge->snaxels[this].erase( s );
			delete s; s = NULL;
			s = s_right;
		}
		stopSnaxel->edge->snaxels[this].erase( stopSnaxel );
		delete stopSnaxel; stopSnaxel = NULL;
	}
	delete [] fixedVP;
}

void SnaxelEvolver::addToMinNV(float c) { 
	minNdotV += c; 
	if(minNdotV <= -1.f) minNdotV = -1.f; 
	else if(minNdotV >= 1.f) minNdotV = 1.f;
}

void SnaxelEvolver::renderSnaxels(bool dashed) {
	glLineStipple(1, 0xFF00);
	if(dashed) glEnable(GL_LINE_STIPPLE);
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonOffset(0, 1);
	glPushMatrix();
	glMultMatrixf( mesh->getTform() );
	Snaxel *s;

	//Calculate occlusions
	if(!dashed) { //Only calculate occlusions if this isn't the dashed-pass
		glColorMask(false,false,false,false);
		glDepthMask(false);
		GLuint occludedFragmentCount, snaxelQuery;
		glGenQueriesARB( 1, &snaxelQuery );
		for(unsigned i = 0; i < snaxelFronts.size(); i++) {
			s = snaxelFronts[i].snaxelHead;
			do {
				glBeginQueryARB( GL_SAMPLES_PASSED_ARB, snaxelQuery );
				glBegin(GL_POINTS);
				float dir[3] = { s->edge->adjVerts[0]->n[0] + s->edge->adjVerts[1]->n[0],
								 s->edge->adjVerts[0]->n[1] + s->edge->adjVerts[1]->n[1], 
								 s->edge->adjVerts[0]->n[2] + s->edge->adjVerts[1]->n[2] };
				normalize(dir);
				glVertex3f(s->p[0]+dir[0]*0.001f, s->p[1]+dir[1]*0.001f, s->p[2]+dir[2]*0.001f);
				glEnd();
				glEndQueryARB( GL_SAMPLES_PASSED_ARB );
				glGetQueryObjectuivARB( snaxelQuery, GL_QUERY_RESULT_ARB, &occludedFragmentCount);
				s->occluded = occludedFragmentCount==0;
				s = s->right;
			} while( s != snaxelFronts[i].snaxelHead );
		}
		glDeleteQueriesARB( 1, &snaxelQuery );
		glColorMask(true,true,true,true);
		glDepthMask(true);
	}
	//Render snaxels
	glLineWidth(4.f);
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		//glColor4fv(this->color);
		glColor3ub(255,0,0);
float t=float(i)/(snaxelFronts.size()>1? float(snaxelFronts.size()-1):1); 
glColor3f(1-t,t,t);
if(i%3==0) glColor3f(1,0,0);
else if(i%3==1) glColor3f(0,1,0);
else if(i%3==2) glColor3f(0,0,1);
		glBegin(GL_LINE_LOOP);
		s = snaxelFronts[i].snaxelHead;
		do {
			glVertex3fv( s->p );
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
		glEnd();
	}
	glLineWidth(1.f);

	glPopMatrix();
	glDisable(GL_BLEND);
	if(dashed) glDisable(GL_LINE_STIPPLE);
}

void SnaxelEvolver::initializeAroundSingleVertex(Vertex *v) {
	snaxelFronts.push_back( SnaxelFront() );
	unsigned size = v->adjEdges.size();
	Snaxel *left_snaxel = NULL;
	for(unsigned j = 0; j < size; j++) {
		Snaxel *s = new Snaxel( int(snaxelFronts.size())-1 );
		s->edge = v->adjEdges[j];
		s->flippedEdge = (v == s->edge->adjVerts[1]);
		float t = 0.5f * s->edge->length;
		s->p[0] = s->edge->adjVerts[0]->p[0] + t * s->edge->direction[0];
		s->p[1] = s->edge->adjVerts[0]->p[1] + t * s->edge->direction[1];
		s->p[2] = s->edge->adjVerts[0]->p[2] + t * s->edge->direction[2];
		s->edge->snaxels[this].insert( s );
		if(left_snaxel) { //Add pointers to the snaxel
			s->left = left_snaxel;
			s->left->right = s;
		} else { //Means this is the head snaxel. Put it in the list struct
			snaxelFronts.back().snaxelHead = s;
		}
		left_snaxel = s;
	}
	//Connect front and back of the list
	snaxelFronts.back().snaxelHead->left = left_snaxel;
	snaxelFronts.back().snaxelHead->left->right = snaxelFronts.back().snaxelHead;
	v->frontsVisited.insert(this); //Add evolver to the visited list
}

void SnaxelEvolver::initializeSnaxelsOrtho(float* vv) {
	float viewvec[3];
	if( fixedVP ) memcpy(viewvec, fixedVP, sizeof(float)*3);
	else memcpy(viewvec, vv, sizeof(float)*3);
	normalize(viewvec); //Normalize view vector
		
	vector<Vertex*> verts = mesh->getVertices();
	float polyNormal[3];
	//Find verts to inialize to
	for(unsigned i = 0; i < verts.size(); i++) {
		//If this vertex or any valence have already been visited by this front, continue
		if( verts[i]->frontsVisited.find(this) != verts[i]->frontsVisited.end() 
												|| verts[i]->occluded ) 
			continue;
		bool valenceInitialized = false;
		for(list<Vertex*>::iterator it = verts[i]->adjVerts.begin();
									it != verts[i]->adjVerts.end(); it++) {
			if( (*it)->frontsVisited.find(this) != (*it)->frontsVisited.end() ) {
				valenceInitialized = true; break; 
			}
		}
		if( valenceInitialized )
			continue;
		//Make sure vertex is oriented correctly
		if( dot(verts[i]->n, viewvec) > 0 ) 
			continue;
		//Go through two edges at a time
		bool positive = false, negative = false;
		unsigned size = verts[i]->adjEdges.size()-1;
		for(unsigned j = 0; j < size; j++) {
			if( positive && negative) break;
			// Check if the view vector lies inside of these normals on the gauss map sphere
			cross(verts[i]->adjEdges[j]->n, verts[i]->adjEdges[j+1]->n, polyNormal); normalize(polyNormal);
			if(dot(viewvec, polyNormal) > 0) positive = true;
			else							negative = true;
		}
		if( (positive && !negative) || (!positive && negative) ) { //positive xor negative
			//Test passed, initialize a new front
			initializeAroundSingleVertex( verts[i] );
		}
	}
	clean(); //Remove spurious snaxels before deformation
}

void SnaxelEvolver::initializeSnaxelsPersp(float* vp) {
	double startTime = getTime();
	//Find verts to inialize to
	vector<Vertex*> verts = mesh->getVertices();
	float polyNormal[3];
	//Allow initialization if a front has not already passed a given vertex
	for(unsigned i = 0; i < verts.size(); i++) {
		//Calculate perspective view vector
		float tmpVp[3];
		if( fixedVP ) memcpy(tmpVp, fixedVP, sizeof(float)*3);
		else memcpy(tmpVp, vp, sizeof(float)*3);
		float vv[3] = { verts[i]->p[0] - tmpVp[0], verts[i]->p[1] - tmpVp[1], 
						verts[i]->p[2] - tmpVp[2] };
		normalize(vv);
		float newndotv = dot(vv,verts[i]->n);
		//only attempt to initialize if no front has passed the vert and if it's front facing (i.e. not occluded)
		bool visited = verts[i]->frontsVisited.find(this) != verts[i]->frontsVisited.end();
		bool frontfacing = newndotv<0.0;
		if( !visited && frontfacing )
			verts[i]->initialize = true;
		else if( visited && !frontfacing )
			verts[i]->initialize = false;
		else
			verts[i]->initialize = false;
		verts[i]->ndotv = newndotv;
	}
	//printf("Setup:  %.8fs\n", getTime()-startTime); startTime = getTime();
	for(unsigned i = 0; i < verts.size(); i++) {
		if( !verts[i]->initialize ) continue;
		//Calculate perspective view vector
		float tmpVp[3];
		if( fixedVP ) memcpy(tmpVp, fixedVP, sizeof(float)*3);
		else memcpy(tmpVp, vp, sizeof(float)*3);
		float vv[3] = { verts[i]->p[0] - tmpVp[0], verts[i]->p[1] - tmpVp[1], 
						verts[i]->p[2] - tmpVp[2] };
		normalize(vv);
if(backfacing) {
		bool minima=true, maxima=true;
		for(list<Vertex*>::iterator it = verts[i]->adjVerts.begin();
									it != verts[i]->adjVerts.end(); it++) {
			if( verts[i]->ndotv > (*it)->ndotv  ) minima = false;
			if( verts[i]->ndotv < (*it)->ndotv  ) maxima = false;
		}
		verts[i]->initialize = minima||maxima;
} else {
		verts[i]->initialize = false;
		//Make sure vertex is oriented correctly
		if( dot(verts[i]->n, vv) > 0 ) continue;
		//Go through two edges at a time
		bool positive = false, negative = false;
		unsigned size = verts[i]->adjEdges.size()-1;
		for(unsigned j = 0; j < size; j++) {
			if( positive && negative) break;
			// Check if the view vector lies inside of these normals on the gauss map sphere
			cross(verts[i]->adjEdges[j]->n, verts[i]->adjEdges[j+1]->n, polyNormal); normalize(polyNormal);
			if(dot(vv, polyNormal) > 0) positive = true;
			else					    negative = true;
		}
		if( (positive && !negative) || (!positive && negative) ) { //positive xor negative
			verts[i]->initialize = true;
		}
}
	}
	//printf("Kernel: %.8fs\n", getTime()-startTime); startTime = getTime();
	for(unsigned i = 0; i < verts.size(); i++) {
		if( verts[i]->initialize ) {
			bool initialize = true;
			for(list<Vertex*>::iterator it = verts[i]->adjVerts.begin();
										it != verts[i]->adjVerts.end(); it++) {
				bool visited = (*it)->frontsVisited.find(this) != (*it)->frontsVisited.end();
				bool frontfacing = (*it)->ndotv<0.0;
				if( visited && frontfacing ) {
					verts[i]->initialize = false; break; 
				}
			}
			if( verts[i]->initialize ) //Test passed, initialize a new front
				initializeAroundSingleVertex( verts[i] );
		}
	}
	//printf("Initia: %.8fs\n", getTime()-startTime); startTime = getTime();
	clean(); //Remove spurious snaxels before deformation
	split();
	merge();
	clean();
	//printf("Clean:  %.8fs\n", getTime()-startTime); startTime = getTime();
}

/* evolve() - Deforms all snaxels one timestep
 * Procedure as follows: 
 *  1) Deform all snaxels, removing snaxels which have less then 'minFrontSize' vertices
 *  2) Split snaxels which are self-intersecting
 *  3) Merge snaxels which have intersected
 *  4) Remove invalid snaxels (cleaning conquest)
 */
#include <cmath>
long int steps=0;
double totDeformTime=0, totCleanTime=0, totSplitTime=0, totMergeTime=0; 
float SnaxelEvolver::evolve(float timestep, float ndotv_coef, float *viewpoint, bool debug, vector<SnaxelEvolver*> *allEvolvers, bool occlusion_pass) {
	double startTime, deformTime, cleanTime, splitTime, mergeTime;
	if(debug) {
		steps++;
		startTime = getTime();
	}
	//Deform
	float avgEnergy = this->deform(timestep, ndotv_coef, viewpoint, allEvolvers, occlusion_pass);
	if(debug) {
		deformTime = getTime()-startTime;
		totDeformTime += deformTime;
		if(steps%10000==0) printf("Deform: %15.10fs [%15.10fs]\n", deformTime, totDeformTime/double(steps));
		startTime = getTime();
	}
	//Clean, Split, Clean, Merge iteratively until we have stable contours
	bool keepGoing;
	do { 
		this->clean(); 
		if(debug) {
			cleanTime = getTime()-startTime;
			totCleanTime += cleanTime;
			if(steps%10000==0) printf("Clean:  %15.10fs [%15.10fs]\n", cleanTime, totCleanTime/double(steps));
			startTime = getTime();
		}
		keepGoing = this->split();  
		if(debug) {
			splitTime = getTime()-startTime;
			totSplitTime += splitTime;
			if(steps%10000==0) printf("Split:  %15.10fs [%15.10fs]\n", splitTime, totSplitTime/double(steps));
			startTime = getTime();
		}
		this->clean();
		if(debug) {
			cleanTime = getTime()-startTime;
			totCleanTime += cleanTime;
			if(steps%10000==0) printf("Clean:  %15.10fs [%15.10fs]\n", cleanTime, totCleanTime/double(steps));
			startTime = getTime();
		}
		keepGoing = this->merge() || keepGoing;
		if(debug) {
			mergeTime = getTime()-startTime;
			totMergeTime += mergeTime;
			if(steps%10000==0) printf("Merge:  %15.10fs [%15.10fs]\n", mergeTime, totMergeTime/double(steps));
			startTime = getTime();
		}
	} while( keepGoing );
	//Total time
	if(debug) {
		double totalTime = deformTime+cleanTime+splitTime+mergeTime;
		double totTotalTime = totDeformTime+totCleanTime+totSplitTime+totMergeTime;
		if(steps%10000==0) printf("Total:  %15.10fs [%15.10fs]\n\n", totalTime, totTotalTime/double(steps));
	}
	//Return the evolver's average energy
	avgEnergy = avgEnergy!=avgEnergy ? 0.f : avgEnergy; //isnan check
	return avgEnergy;
}

float SnaxelEvolver::deform(float timestep, float ndotv, float* viewpoint, vector<SnaxelEvolver*> *allEvolvers, bool occlusion_pass) {
	//Deformation - evolve snaxels and fan out
	Vertex *to, *from;
	float energy = 0; int totalSnaxels = 0; //calculate and return average energy^2 for all snaxels
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do {
			//Continue if this deformation is an occlusion pass but s isn't occluded
			if( occlusion_pass && !s->occluded ) {
				s = s->right;
				continue;
			}
			//If another snaxel is already on this edge (collision), freeze the snaxel
			if( s->edge->snaxels[this].size() > 1 ) {
				s = s->right;
				continue;
			}
			//Orient correctly
			if(s->flippedEdge) {
				to = s->edge->adjVerts[0]; from = s->edge->adjVerts[1];
			} else {
				to = s->edge->adjVerts[1]; from = s->edge->adjVerts[0];
			}
			//Interpolate edge verts for approximating normal
			float dist_to = dist3(s->p, to->p);
			float dist_from = s->edge->length - dist_to;
			dist_to /= s->edge->length;
			dist_from /= s->edge->length;
			//Calculate velocity
			float viewvec[3] = { s->p[0]-viewpoint[0], s->p[1]-viewpoint[1], s->p[2]-viewpoint[2] };
			normalize(viewvec);
			float fromNormDotVV = dot(from->n,viewvec);
			float toNormDotVV = dot(to->n,viewvec);
			//float viewTerm = dist_to*fromNormDotVV + dist_from*toNormDotVV;
			
			float theta_ = acos(dot(from->n,to->n));
			float ffrom = sin(dist_to*theta_)/sin(theta_);
			float fto = sin(dist_from*theta_)/sin(theta_);
			float NN[3] = {ffrom*from->n[0]+fto*to->n[0], ffrom*from->n[1]+fto*to->n[1], ffrom*from->n[2]+fto*to->n[2]};
			float viewTerm = dot(NN,viewvec);
			
			if( fixedVP ) {
				float lightvec[3] = { s->p[0]-fixedVP[0], s->p[1]-fixedVP[1], s->p[2]-fixedVP[2] };
				normalize(lightvec);
				float fromNormDotLV = dot(from->n,lightvec);
				float toNormDotLV = dot(to->n,lightvec);
				float lightTerm = minNdotV + dist_to*fromNormDotLV + dist_from*toNormDotLV;
				viewTerm = max(viewTerm, lightTerm);
			}
			float velocity = timestep * ndotv * -viewTerm;

//Snaxels for planar map
/*if(allEvolvers) {
float tmp_sn1[3] = {s->left->u[0]-s->u[0], s->left->u[1]-s->u[1], 0};
normalize(tmp_sn1);
float sn1[2] = {tmp_sn1[1], -tmp_sn1[0]};
float tmp_sn2[3] = {s->u[0]-s->right->u[0], s->u[1]-s->right->u[1], 0};
normalize(tmp_sn2);
float sn2[2] = {tmp_sn2[1], -tmp_sn2[0]};
float sn[3] = {sn1[0]+sn2[0], sn1[1]+sn2[1], 0};
normalize(sn); //Snaxel normal
bool pm = false;
Snaxel *nearest2dSnaxel;
for(unsigned b = 0; b < (*allEvolvers).size(); b++) {
	vector<SnaxelFront>* fronts = (*allEvolvers)[b]->getSnaxelFronts();
	for(unsigned a = 0; a < (*fronts).size(); a++) {
		if( fronts==&snaxelFronts && a==i) {
			float p2d[2] = {s->u[0] + sn[0]*EPSILON, s->u[1] + sn[1]*EPSILON};
			if((*fronts)[a].isInsideFront2d(p2d, &nearest2dSnaxel)) {
				//pm = true; 
				break;
			} 
		} else {
			if((*fronts)[a].isInsideFront2d(s->u, &nearest2dSnaxel) && s->depth > nearest2dSnaxel->depth ) {
			//	pm = true; 
				break;
			} 
		}
	}
}
if(pm || s->occluded) velocity = velocity*-1;
}
*/
			if(backfacing) velocity = -velocity;
			//For calculating iteration's average energy:
			energy += velocity*velocity;
			totalSnaxels++;
			if( s->flippedEdge ) velocity = -velocity; //Flip depending on edge orientation
			if( occlusion_pass && s->occluded ) velocity = -velocity;
			//Store velocity, calculate new acceleration
			s->acceleration = velocity - s->velocity; //new velocity - old velocity
			s->velocity = velocity;
			//Get new position
			s->p[0] += velocity*s->edge->direction[0];
			s->p[1] += velocity*s->edge->direction[1];
			s->p[2] += velocity*s->edge->direction[2];
			//If the snaxel hits a vertex, fan out
			if( dist3(s->p, from->p) > s->edge->length ) { //Deforming forwards
				//Find starting spot
				unsigned size = to->adjEdges.size();
				unsigned start;
				for(unsigned j = 0; j < size; j++) {
					if( s->edge == to->adjEdges[j] ) {
						start = j;
						break;
					}
				}
				//Store the current left and right snaxels for later insertion
				Snaxel *left_snaxel = s->left;
				Snaxel *right_snaxel = s->right;
				//Erase current snaxel
				left_snaxel->right = right_snaxel;
				right_snaxel->left = left_snaxel;
				if( s == snaxelFronts[i].snaxelHead )
					snaxelFronts[i].snaxelHead = right_snaxel->left;
				s->edge->snaxels[this].erase( s );
				Snaxel* parent = s->parent;
				delete s; s = NULL;
				//Initialize new snaxels
				for(int j = 1; j < (int)size; j++) {
					int idx = (int(j+start)) % int(size);
					Snaxel *s_new = new Snaxel((int)i);
					s_new->edge = to->adjEdges[idx];
					s_new->flippedEdge = (to == s_new->edge->adjVerts[1]);
					float t = 0.0001f * s_new->edge->length;
					if( !s_new->flippedEdge ) {
						s_new->p[0] = s_new->edge->adjVerts[0]->p[0] + t * s_new->edge->direction[0];
						s_new->p[1] = s_new->edge->adjVerts[0]->p[1] + t * s_new->edge->direction[1];
						s_new->p[2] = s_new->edge->adjVerts[0]->p[2] + t * s_new->edge->direction[2];
					} else {
						s_new->p[0] = s_new->edge->adjVerts[1]->p[0] - t * s_new->edge->direction[0];
						s_new->p[1] = s_new->edge->adjVerts[1]->p[1] - t * s_new->edge->direction[1];
						s_new->p[2] = s_new->edge->adjVerts[1]->p[2] - t * s_new->edge->direction[2];
					}
					s_new->edge->snaxels[this].insert( s_new );
					//Fix pointers
					s_new->left = left_snaxel;
					s_new->left->right = s_new;
					left_snaxel = s_new;
					//Copy parents
					s_new->parent = parent;
				}
				//Fix pointers
				left_snaxel->right = right_snaxel;
				left_snaxel->right->left = left_snaxel;
				to->frontsVisited.insert(this); //Add evolver to the visited list
				s = left_snaxel;
			} else if( dist3(s->p, to->p) > s->edge->length ) { //Deforming backwards
				//Find starting spot
				unsigned size = from->adjEdges.size();
				unsigned start;
				for(unsigned j = 0; j < size; j++) {
					if( s->edge == from->adjEdges[j] ) {
						start = j;
						break;
					}
				}
				//Store the current left and right snaxels for later insertion
				Snaxel *left_snaxel = s->left;
				Snaxel *right_snaxel = s->right;
				//Erase current snaxel
				left_snaxel->right = right_snaxel;
				right_snaxel->left = left_snaxel;
				if( s == snaxelFronts[i].snaxelHead )
					snaxelFronts[i].snaxelHead = right_snaxel->left;
				s->edge->snaxels[this].erase( s );
				Snaxel* parent = s->parent;
				delete s; s = NULL;
				//Initialize new snaxels
				for(int j = (int)size-1; j >= 1; j--) {
					int idx = (int(j+start)) % int(size);
					Snaxel *s_new = new Snaxel((int)i);
					s_new->edge = from->adjEdges[idx];
					s_new->flippedEdge = (from == s_new->edge->adjVerts[0]);
					float t = 0.001f * s_new->edge->length;
					if( s_new->flippedEdge ) {
						s_new->p[0] = s_new->edge->adjVerts[0]->p[0] + t * s_new->edge->direction[0];
						s_new->p[1] = s_new->edge->adjVerts[0]->p[1] + t * s_new->edge->direction[1];
						s_new->p[2] = s_new->edge->adjVerts[0]->p[2] + t * s_new->edge->direction[2];
					} else {
						s_new->p[0] = s_new->edge->adjVerts[1]->p[0] - t * s_new->edge->direction[0];
						s_new->p[1] = s_new->edge->adjVerts[1]->p[1] - t * s_new->edge->direction[1];
						s_new->p[2] = s_new->edge->adjVerts[1]->p[2] - t * s_new->edge->direction[2];
					}
					s_new->edge->snaxels[this].insert( s_new );
					//Fix pointers
					s_new->left = left_snaxel;
					s_new->left->right = s_new;
					from->frontsVisited.erase(this); //Remove from visited list
					left_snaxel = s_new;
					//Copy parents
					s_new->parent = parent;
				}
				//Fix pointers
				left_snaxel->right = right_snaxel;
				left_snaxel->right->left = left_snaxel;
				s = left_snaxel;
			}
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
	return energy/float(totalSnaxels);
}

bool SnaxelEvolver::isInsideFront(const SnaxelFront& sf) {
	Snaxel *s = sf.snaxelHead;
	bool isInside = true;
	do {
		Vertex *to = s->flippedEdge ? s->edge->adjVerts[0] : s->edge->adjVerts[1];
		set<SnaxelEvolver*>::iterator se = to->frontsVisited.find(this);
		if( se == to->frontsVisited.end() || *to->frontsVisited.find(this) != this ) {
			isInside = false; break;
		}
		s = s->right;
	} while(s != sf.snaxelHead );
	return isInside;
}

void SnaxelEvolver::clean() {
	//Cleaning - remove adjacent snaxels on same edge
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do {
			if( (s->edge == s->right->edge) ) {
				//Remove both snaxels
				Snaxel *s_right_right = s->right->right;
				s->left->right = s_right_right;
				s_right_right->left = s->left;
				if( s == snaxelFronts[i].snaxelHead )
					snaxelFronts[i].snaxelHead = s_right_right->left;
				else if( s->right == snaxelFronts[i].snaxelHead ) 
					snaxelFronts[i].snaxelHead = s_right_right;
				s->edge->snaxels[this].erase( s );
				s->right->edge->snaxels[this].erase( s->right );
				//Copy s parent to s->right->right
				s->right->right->parent = s->parent;
				//Remove ptrs
				delete s->right; s->right = NULL;
				delete s; s = NULL;
				if( s_right_right && s_right_right->left ) s = s_right_right->left;
				else {
					snaxelFronts.erase( snaxelFronts.begin() + i-- );
					break;
				}
			}
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
	//Cleaning - remove snaxel if on filled face (adjacent snaxels on all 3 face edges)
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		snaxelFronts[i].size = 0; //Count how many are in the current list
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do {
			if(( (s->edge->adjFaces[0] == s->left->edge->adjFaces[0] ||
				  s->edge->adjFaces[0] == s->left->edge->adjFaces[1]) &&
				 (s->edge->adjFaces[0] == s->right->edge->adjFaces[0] ||
				  s->edge->adjFaces[0] == s->right->edge->adjFaces[1])   ) || 
			   ( (s->edge->adjFaces[1] == s->left->edge->adjFaces[0] ||
				  s->edge->adjFaces[1] == s->left->edge->adjFaces[1]) &&
				 (s->edge->adjFaces[1] == s->right->edge->adjFaces[0] ||
				  s->edge->adjFaces[1] == s->right->edge->adjFaces[1])   ))
			{
				Snaxel *s_right = s->right;
				s->right->left = s->left;
				s->left->right = s->right;
				if( s == snaxelFronts[i].snaxelHead )
					snaxelFronts[i].snaxelHead = s_right->left;
				s->edge->snaxels[this].erase( s );
				//Copy s parents to s->right
				s->right->parent = s->parent;
				//Remove ptrs
				delete s; s = NULL;
				snaxelFronts[i].size--;
				if( s_right && s_right->left ) s = s_right->left;
				else {
					snaxelFronts.erase( snaxelFronts.begin() + i-- );
					break;
				}
			}
			snaxelFronts[i].size++;
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
	//Remove lists that have less than 'minFrontSize' snaxels
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		if( snaxelFronts[i].size < minFrontSize ) {//|| isInsideFront(snaxelFronts[i]) ) {
			Snaxel *stopSnaxel = snaxelFronts[i].snaxelHead->left;
			while( s != stopSnaxel ) {
				Snaxel *s_right = s->right;
				s->edge->snaxels[this].erase( s );
				delete s; s = NULL;
				s = s_right;
			}
			stopSnaxel->edge->snaxels[this].erase( stopSnaxel );
			delete stopSnaxel; stopSnaxel = NULL;
			snaxelFronts.erase( snaxelFronts.begin() + i );
			i--;
		} else {
			//And fix snaxel list indices while we're at it
			do { 
				s->listIdx = i;
				s = s->right;
			} while( s != snaxelFronts[i].snaxelHead );
		}
	}
}

bool SnaxelEvolver::split() {
	bool somethingSplit = false;
	bool fixListIdxs = false; //Only need to fix indices if an entire list is deleted
	//Splitting - where self intersection occurs, split into two snaxel lists
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		//Check if any snaxels lie on the same edge (only for this snaxel list)
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do { 
			if( s->edge->snaxels[this].size() > 1 ) {
				//Find two snaxels from the same list (if they exist)
				Snaxel *start = s, *end = NULL;
				for(set<Snaxel*>::iterator it = start->edge->snaxels[this].begin();
										   it != start->edge->snaxels[this].end(); it++ ) {
					if( start != (*it) && start->listIdx == (*it)->listIdx ) {
						end = *it; break;
					}
				}
				//Make two lists from one if self intersection exists
				if( end ) {
					//See how many snaxels between start and end
					Snaxel *s_backward = start, *s_forward = start;
					int s_count = 0; 
					bool delete_backward = false, delete_forward = false;
					bool fixHead = (start == snaxelFronts[i].snaxelHead);
					do {
						if( s_backward == end ) delete_backward = true;
						if( s_forward == end ) delete_forward = true;
						if( s_backward == snaxelFronts[i].snaxelHead || 
							s_forward == snaxelFronts[i].snaxelHead )
							fixHead = true;
						s_backward = s_backward->left;
						s_forward = s_forward->right;
					} while( s_count++ < minFrontSize+2 );
					if( delete_backward && delete_forward ) { //Too small for split, delete entire list
						Snaxel *ss = snaxelFronts[i].snaxelHead;
						Snaxel *stopSnaxel = snaxelFronts[i].snaxelHead->left;
						while( ss != stopSnaxel ) {
							Snaxel *ss_right = ss->right;
							Vertex *to_v = (ss->flippedEdge ? ss->edge->adjVerts[0] : ss->edge->adjVerts[1]);
							to_v->frontsVisited.insert(this); //Add evolver to the visited list
							ss->edge->snaxels[this].erase( ss );
							delete ss; ss = NULL;
							ss = ss_right;
						}
						Vertex *to_v = (stopSnaxel->flippedEdge ?
										stopSnaxel->edge->adjVerts[0] : stopSnaxel->edge->adjVerts[1]);
						to_v->frontsVisited.insert(this); //Add evolver to the visited list
						stopSnaxel->edge->snaxels[this].erase( stopSnaxel );
						delete stopSnaxel; stopSnaxel = NULL;
						snaxelFronts.erase( snaxelFronts.begin() + i );
						i--;
						fixListIdxs = true;
						break;
					} else if( delete_backward ) { //Too small for split, delete end->start
						//Fix pointers
						start->right->left = end->left;
						end->left->right = start->right;
						if(fixHead)
							snaxelFronts[i].snaxelHead = end->left;
						s = end->left->left;
						//Remove snaxels (end to start)
						Snaxel *stopSnaxel = end->left;
						Snaxel *ss = start;
						do {
							Snaxel *ss_left = ss->left;
							Vertex *to_v = (ss->flippedEdge ? ss->edge->adjVerts[0] : ss->edge->adjVerts[1]);
							to_v->frontsVisited.insert(this); //Add evolver to the visited list
							ss->edge->snaxels[this].erase( ss );
							delete ss; ss = NULL;
							ss = ss_left;
						} while( ss != stopSnaxel );
					} else if( delete_forward ) { //Too small for split, delete start->end
						//Fix pointers
						start->left->right = end->right;
						end->right->left = start->left;
						if(fixHead)
							snaxelFronts[i].snaxelHead = start->left;
						s = start->left->left;
						//Remove snaxels (start to end)
						Snaxel *stopSnaxel = end->right;
						Snaxel *ss = start;
						do {
							Snaxel *ss_right = ss->right;
							Vertex *to_v = (ss->flippedEdge ? ss->edge->adjVerts[0] : ss->edge->adjVerts[1]);
							to_v->frontsVisited.insert(this); //Add evolver to the visited list
							ss->edge->snaxels[this].erase( ss );
							delete ss; ss = NULL;
							ss = ss_right;
						} while( ss != stopSnaxel );
					} else { //Split
						//Change in topology, mark as ridge/valley (k1*k2 == gaussian curvature == 0)
						Edge* edge = start->edge;
						edge->ridge = true;
						edge->ridge_p[0] = 0.5f*(start->p[0]+end->p[0]);
						edge->ridge_p[1] = 0.5f*(start->p[1]+end->p[1]);
						edge->ridge_p[2] = 0.5f*(start->p[2]+end->p[2]);
						//Create new list, fix pointers
						snaxelFronts.push_back( SnaxelFront() );
						start->left->right = end->right;
						start->right->left = end->left;
						end->left->right = start->right;
						end->right->left = start->left;
						//Fix head snaxel pointers
						snaxelFronts[i].snaxelHead = start->left;
						snaxelFronts.back().snaxelHead = start->right;
						//Delete start and end
						start->edge->snaxels[this].erase( start );
						end->edge->snaxels[this].erase( end );
						delete start; start = NULL;
						delete end; end = NULL;
						//Fix list indices
						Snaxel *ss = snaxelFronts.back().snaxelHead;
						int backIdx = snaxelFronts.size() - 1;
						do {
							ss->listIdx = backIdx;
							ss = ss->right;
						} while( ss != snaxelFronts.back().snaxelHead );
						//Decrement loop to check same list for another collision
						//i--;
						break;
					}
					somethingSplit = true;
				}
			}
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
	//A list was deleted, assign correct list indices
	if( fixListIdxs ) {
		for(unsigned i = 0; i < snaxelFronts.size(); i++) {
			Snaxel *s = snaxelFronts[i].snaxelHead;
			snaxelFronts[i].size = 0;
			do {
				snaxelFronts[i].size++;
				s->listIdx = int(i);
				s = s->right;
			} while( s!=snaxelFronts[i].snaxelHead);
		}
	}
	return somethingSplit;
}

bool SnaxelEvolver::merge() {
	//Merging - where global intersections occur, merge into one snaxel list
	bool somethingMerged = false;
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		//Check if any snaxels lie on the same edge
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do { 
			if( s->edge->snaxels[this].size() > 1 ) {
				//Find two snaxels from different lists (if they exist)
				Snaxel *start = s, *end = NULL;
				for(set<Snaxel*>::iterator it = start->edge->snaxels[this].begin();
										   it != start->edge->snaxels[this].end(); it++ ) {
					if( start->listIdx != (*it)->listIdx ) {
						end = *it; break;
					}
				}
				if( end ) {
					bool keepStart = (start->listIdx <= end->listIdx); 
					int keptListIdx = keepStart ? start->listIdx : end->listIdx; //Keep lowest index
					int discardedListIdx = keepStart ? end->listIdx : start->listIdx; //Discard highest
//printf("Keep %d, discard %d, total %d\n", keptListIdx, discardedListIdx, snaxelFronts.size());
					//Fix list indices of discarded list
					Snaxel *ss = snaxelFronts[discardedListIdx].snaxelHead;
					do {
						ss->listIdx = keptListIdx;
						ss = ss->right;
					} while( ss != snaxelFronts[discardedListIdx].snaxelHead );
					//Fix pointers
					start->left->right = end->right;
					start->right->left = end->left;
					end->left->right = start->right;
					end->right->left = start->left;
					//Set new head snaxel
					Snaxel *start_left = start->left;
					if( start == snaxelFronts[start->listIdx].snaxelHead ||
						end == snaxelFronts[end->listIdx].snaxelHead ) {
						snaxelFronts[keptListIdx].snaxelHead = start_left;
					}
					//Delete start and end
					start->edge->snaxels[this].erase( start );
					end->edge->snaxels[this].erase( end );
					delete start; start = NULL;
					delete end; end = NULL;
					//Discard the merged list
					snaxelFronts.erase( snaxelFronts.begin() + discardedListIdx );
					//Fix all list indices following the discarded list
					for(unsigned j = discardedListIdx; j < snaxelFronts.size(); j++) {
						ss = snaxelFronts[j].snaxelHead;
						do { 
							ss->listIdx = j;
							ss = ss->right;
						} while( ss != snaxelFronts[j].snaxelHead );
					}
					somethingMerged = true;
					//i--;
					break;
				}
			}
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
	return somethingMerged;
}

//Same as evolve, except only moves occluded snaxels so they are visible
void SnaxelEvolver::removeOcclusions(float timestep, float ndotv_coef, float* viewpoint, bool debug) {
	//Updates only snaxels who are still occluded. Stops when all snaxels are visible.
	while( calculateSnaxelOcclusion() ) { //calculateRemainingOcclusions(...) returns true if any snaxels are occluded
		this->evolve(timestep, ndotv_coef, viewpoint, debug, NULL, true);
	}
}

//Calculate 2D projected positions of snaxels
void SnaxelEvolver::projectSnaxels(float *projMatrix, float win_width, float win_height) {
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		int idx = 0;
		do {
			float norm = projMatrix[3]*s->p[0] + projMatrix[7]*s->p[1] + projMatrix[11]*s->p[2] + projMatrix[15];
			s->u[0] = (projMatrix[0]*s->p[0] + projMatrix[4]*s->p[1] + projMatrix[8]*s->p[2] + projMatrix[12]) / norm;
			s->u[0] = 0.5f*(s->u[0]+1.f)*win_width;
			s->u[1] = (projMatrix[1]*s->p[0] + projMatrix[5]*s->p[1] + projMatrix[9]*s->p[2] + projMatrix[13]) / norm;
			s->u[1] = 0.5f*(-s->u[1]+1.f)*win_height; //- to flip y orientation
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
}

//Calculate the average depth of the fronts (distance to camera), sorts them back to front
bool sortFrontDepth(SnaxelFront f1, SnaxelFront f2) { return f1.depth>f2.depth; };
void SnaxelEvolver::calculateFrontDepthAndSort(float* projMatrix) {
	//Depth calculation
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		int size = 0;
		float centroid[3]={0,0,0};
		do {
			float s_norm = projMatrix[3]*s->p[0] + projMatrix[7]*s->p[1] + projMatrix[11]*s->p[2] + projMatrix[15];
			s->depth = (projMatrix[2]*s->p[0] + projMatrix[6]*s->p[1] + projMatrix[10]*s->p[2] + projMatrix[14]) / s_norm;
			centroid[0] += s->p[0];
			centroid[1] += s->p[1];
			centroid[2] += s->p[2];
			size++;
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
		centroid[0] /= float(size);
		centroid[1] /= float(size);
		centroid[2] /= float(size);
		float norm = projMatrix[3]*centroid[0] + projMatrix[7]*centroid[1] + projMatrix[11]*centroid[2] + projMatrix[15];
		snaxelFronts[i].depth = (projMatrix[2]*centroid[0] + projMatrix[6]*centroid[1] + projMatrix[10]*centroid[2] + projMatrix[14]) / norm;
	}
	//Sort back->front
	//sort(snaxelFronts.begin(), snaxelFronts.end(), SnaxelFront());
	sort(snaxelFronts.begin(), snaxelFronts.end(), sortFrontDepth);
}

//Update head snaxels so that they are near previous head snaxels (for coherent contours)
void SnaxelEvolver::makeHeadsCoherent() {
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		float min_dist = FLT_MAX;
		Snaxel *newhead = NULL;
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do {
			//float d = dist2(snaxelFronts[i].lastHeadUv, s->u);
			float d = dist3(snaxelFronts[i].lastHeadP, s->p);
			if( d < min_dist ) {
				min_dist = d;
				newhead = s;
			}
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
		snaxelFronts[i].snaxelHead = newhead;
		//if( !lastHeadInitialized ) {
			snaxelFronts[i].lastHeadUv[0] = newhead->u[0];
			snaxelFronts[i].lastHeadUv[1] = newhead->u[1];
			snaxelFronts[i].lastHeadP[0] = newhead->p[0];
			snaxelFronts[i].lastHeadP[1] = newhead->p[1];
			snaxelFronts[i].lastHeadP[2] = newhead->p[2];
			lastHeadInitialized = true;
		//}
	}
}

void SnaxelEvolver::get3dSilhouettes( vector< vector<Vertex> > *ps, float intrvl, float off_intrvl, bool find_pts ) {
	//bool find_pts = true => ps returns points on silhouette rather than stippled lines
	ps->clear();
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		float curpos = 0.f;
		bool make_line = true;
		ps->push_back( vector<Vertex>() );
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do {
			if( !find_pts && make_line ) {
				Vertex v; 
				memcpy(v.p, s->p, 3*sizeof(float));
				ps->back().push_back( v );
			}
			float dist = dist3(s->p, s->right->p);
			float dir[3] = { (s->right->p[0]-s->p[0])/dist, (s->right->p[1]-s->p[1])/dist, (s->right->p[2]-s->p[2])/dist };
			float t = curpos;
			while( dist-t > 0.f ) {
				Vertex v; 
				v.p[0] = s->p[0] + t*dir[0];
				v.p[1] = s->p[1] + t*dir[1];
				v.p[2] = s->p[2] + t*dir[2];
				ps->back().push_back( v );
				if( find_pts ) {
					t += intrvl;
				} else if(make_line) {
					t += off_intrvl;
					ps->push_back( vector<Vertex>() );
					make_line = false;
				} else {
					t += intrvl;
					make_line = true;
				}
			}
			curpos = t-dist;
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
		if( !find_pts && make_line ) {
			Vertex v; 
			memcpy(v.p, s->p, 3*sizeof(float));
			ps->back().push_back( v );
		}
	}
}

//Find fronts that should be rendered as "holes" in the planar map/SVG
//Currently unused!! see commented out line near bottom of function
void SnaxelEvolver::calculateHoleFronts(float *projMatrix) {
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		Snaxel *s = snaxelFronts[i].snaxelHead;
		do { //Find an unoccluded snaxel
			if(!s->occluded) break;
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
		if(s->occluded) {
			snaxelFronts[i].holeContour = false;
			continue;
		}
		float normal[3] = {0.5f*(s->edge->adjVerts[0]->n[0]+s->edge->adjVerts[1]->n[0]),
						   0.5f*(s->edge->adjVerts[0]->n[1]+s->edge->adjVerts[1]->n[1]),
						   0.5f*(s->edge->adjVerts[0]->n[2]+s->edge->adjVerts[1]->n[2])};
		normalize(normal);
		float norm = projMatrix[3]*normal[0] + projMatrix[7]*normal[1] + projMatrix[11]*normal[2] + projMatrix[15];
		float nu =  (projMatrix[0]*normal[0] + projMatrix[4]*normal[1] + projMatrix[ 8]*normal[2] + projMatrix[12]) / norm;
		float nv =  (projMatrix[1]*normal[0] + projMatrix[5]*normal[1] + projMatrix[ 9]*normal[2] + projMatrix[13]) / norm;
		float u[2] = {EPSILON*nu + s->u[0], EPSILON*nv + s->u[1]};
		Snaxel *nearest2dSnaxel;
		snaxelFronts[i].holeContour = snaxelFronts[i].isInsideFront2d(u, &nearest2dSnaxel);
		//printf("%d -> hole: %s\n", i, snaxelFronts[i].holeContour?"yes":"no");
snaxelFronts[i].holeContour = false;
	}
	//printf("\n");
}

void SnaxelEvolver::initializeParents(SnaxelEvolver* copyOfEvolver) {
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		(*copyOfEvolver->getSnaxelFronts()).push_back(SnaxelFront());
		Snaxel *s = snaxelFronts[i].snaxelHead;
		Snaxel *s_copy = new Snaxel(s);
		(*copyOfEvolver->getSnaxelFronts())[i].snaxelHead = s_copy;
		(*copyOfEvolver->getSnaxelFronts())[i].size = snaxelFronts[i].size;
		s->parent = s_copy;
		do {
			s = s->right;
			Snaxel *par = new Snaxel(s);
			s->parent = par;
			s_copy->right = par;
			s_copy->right->left = s_copy;
			s_copy = s_copy->right;
		} while( s->right != snaxelFronts[i].snaxelHead );
		s_copy->right = (*copyOfEvolver->getSnaxelFronts())[i].snaxelHead;
		s_copy->right->left = s_copy;
	}
}

//Sets the occluded flag for all snaxels
bool SnaxelEvolver::calculateSnaxelOcclusion() {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(5, 5);
	glPushMatrix();
	glMultMatrixf( mesh->getTform() );
	Snaxel *s;
	bool anyOccluded = false;
	//Calculate occlusions
	glColorMask(false,false,false,false);
	glDepthMask(false);
	GLuint occludedFragmentCount, snaxelQuery;
    glGenQueriesARB( 1, &snaxelQuery );
	for(unsigned i = 0; i < snaxelFronts.size(); i++) {
		s = snaxelFronts[i].snaxelHead;
		do {
			glBeginQueryARB( GL_SAMPLES_PASSED_ARB, snaxelQuery );
			glBegin(GL_POINTS);
			float dir[3] = { s->edge->adjVerts[0]->n[0] + s->edge->adjVerts[1]->n[0],
							 s->edge->adjVerts[0]->n[1] + s->edge->adjVerts[1]->n[1], 
							 s->edge->adjVerts[0]->n[2] + s->edge->adjVerts[1]->n[2] };
			normalize(dir);
			glVertex3f(s->p[0]+dir[0]*0.001f, s->p[1]+dir[1]*0.001f, s->p[2]+dir[2]*0.001f);
			glEnd();
			glEndQueryARB( GL_SAMPLES_PASSED_ARB );
			glGetQueryObjectuivARB( snaxelQuery, GL_QUERY_RESULT_ARB, &occludedFragmentCount);
			s->occluded = occludedFragmentCount==0;
			anyOccluded = s->occluded || anyOccluded; //anyOccluded==true if any snaxel is occluded
			s = s->right;
		} while( s != snaxelFronts[i].snaxelHead );
	}
    glDeleteQueriesARB( 1, &snaxelQuery );
	glColorMask(true,true,true,true);
	glDepthMask(true);
	glPopMatrix();
	//If any snaxels are still occluded, return true
	return anyOccluded;
}
