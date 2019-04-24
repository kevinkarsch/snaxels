#include "PlanarMapDetector.h"

PlanarMapDetector::PlanarMapDetector() {
}

PlanarMapDetector::~PlanarMapDetector() {
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
float *getColor(int i, float *c) {
	c[0] = float(rand()) / float(RAND_MAX);
	c[1] = float(rand()) / float(RAND_MAX);
	c[2] = float(rand()) / float(RAND_MAX);
	return c;
}

vector<gpc_polygon>* PlanarMapDetector::findPlanarMap(vector<SnaxelEvolver*>& evols, float *P, vector<SnaxelEvolver*>** evolsOut, int w, int h) {
	//Pre-calculate 2D positions, create GPC polygons
	gpc_polygon all_union = gpc_make_null_poly(); //All polygons unioned together
	vector<gpc_polygon> polygons; //Each front/evolver turned into a 2D polygon
	Snaxel *s;
	for(int i = 0; i < int(evols.size()); i++) {
		vector<SnaxelFront>* fronts = evols[i]->getSnaxelFronts();	
		for(int j = 0; j < int(fronts->size()); j++) {
			gpc_vertex_list contour;
			contour.num_vertices = 0;
			contour.vertex = NULL;
			s = (*fronts)[j].snaxelHead;
			int idx = 0;
			do {
				float norm = P[3]*s->p[0] + P[7]*s->p[1] + P[11]*s->p[2] + P[15];
				s->u[0] = (P[0]*s->p[0] + P[4]*s->p[1] + P[8]*s->p[2] + P[12]) / norm;
				s->u[1] = (P[1]*s->p[0] + P[5]*s->p[1] + P[9]*s->p[2] + P[13]) / norm;

				contour.vertex = (gpc_vertex*)realloc(contour.vertex, 
									sizeof(gpc_vertex)*(++contour.num_vertices));
				contour.vertex[contour.num_vertices-1].x = s->u[0];
				contour.vertex[contour.num_vertices-1].y = s->u[1];

				s = s->right;
			} while( s != (*fronts)[j].snaxelHead );

			//Remove any complexities from the resulting contour
			gpc_polygon* simple_polygons = NULL;
			int num_simple_polys = 0;
			gpc_fix_self_intersections( &contour, &simple_polygons, &num_simple_polys ); 

			//Insert resulting polygons, add to the current total union
			for(int k = 0; k < num_simple_polys; k++) {
				polygons.push_back( simple_polygons[k] );
				gpc_polygon tmp_all_union = gpc_make_null_poly();
				gpc_polygon_clip(GPC_UNION, &all_union, &simple_polygons[k], &tmp_all_union);
				all_union = tmp_all_union;
			}
			free(simple_polygons);
		}
	}
	//Clip into planar maps
//TODO: fix self intersection problem (ex: ribbon) - need to get union working (and/or remove contour self intersections) 
//TODO: fix genus > 0 problems (ex: torus) - can find this during snaxel self intersections (phase 3)
	vector<gpc_polygon>* map_polygons = new vector<gpc_polygon>;
	if( ! polygons.empty() ) {
		//First, exclusive or all polygons together
		gpc_polygon gxor = polygons[0];
		for(unsigned i = 1; i < polygons.size(); i++) {
			gpc_polygon xor_tmp = gpc_make_null_poly();
			gpc_polygon_clip(GPC_XOR, &gxor, &(polygons[i]), &xor_tmp);
			gxor = xor_tmp;
		}
		map_polygons->push_back( gxor );
		//Then find the inverse wrt the union of all polygons
		gpc_polygon union_diff_xor = gpc_make_null_poly();
		gpc_polygon_clip(GPC_DIFF, &all_union, &gxor, &union_diff_xor);
		map_polygons->push_back( union_diff_xor );

		//testing (makes more polys than necessary)
		// whole polys
		for(unsigned i = 0; i < polygons.size(); i++) {
			map_polygons->push_back( polygons[i] );
		}
		if(polygons.size()>1) {
			// A\(union rest)
			for(unsigned i = 0; i < polygons.size(); i++) {
				gpc_polygon lhs = polygons[i];
				gpc_polygon rhs = gpc_make_null_poly();
				for(unsigned j = 0; j < polygons.size(); j++) {
					if(j==i) continue;
					gpc_polygon rhs_tmp = gpc_make_null_poly();
					gpc_polygon_clip(GPC_UNION, &rhs, &polygons[j], &rhs_tmp);
					rhs = rhs_tmp;
				}
				gpc_polygon out_tmp = gpc_make_null_poly();
				gpc_polygon_clip(GPC_DIFF, &lhs, &rhs, &out_tmp);
				map_polygons->push_back( out_tmp );
			}
			//A int B \ (union rest)
			for(unsigned i = 0; i < polygons.size(); i++) {
				for(unsigned k = 0; k < polygons.size(); k++) {
					if(i==k) continue;
					gpc_polygon lhsi = polygons[i];
					gpc_polygon lhsk = polygons[k];
					gpc_polygon lhs = gpc_make_null_poly();
					gpc_polygon_clip(GPC_INT, &lhsi, &lhsk, &lhs);
					gpc_polygon rhs = gpc_make_null_poly();
					for(unsigned j = 0; j < polygons.size(); j++) {
						if(j==i || j==k) continue;
						gpc_polygon rhs_tmp = gpc_make_null_poly();
						gpc_polygon_clip(GPC_UNION, &rhs, &polygons[j], &rhs_tmp);
						rhs = rhs_tmp;
					}
					gpc_polygon out_tmp = gpc_make_null_poly();
					gpc_polygon_clip(GPC_DIFF, &lhs, &rhs, &out_tmp);
					map_polygons->push_back( out_tmp );
				}
			}
			//intersect all
			gpc_polygon ginters = polygons[0];
			for(unsigned i = 1; i < polygons.size(); i++) {
				gpc_polygon ginters_tmp = gpc_make_null_poly();
				gpc_polygon_clip(GPC_INT, &ginters, &(polygons[i]), &ginters_tmp);
				ginters = ginters_tmp;
			}
			map_polygons->push_back( ginters );
		}
	}

	//Convert to evolvers
	float color[4]={0,0,0,1};
	int cIdx=0;
	srand( time(NULL) );
	if(evolsOut) {
		(*evolsOut) = new vector<SnaxelEvolver*>;
		for(int i=0; i < (*map_polygons).size(); i++) {
			for(int j=0; j < (*map_polygons)[i].num_contours; j++) {
				(**evolsOut).push_back( new SnaxelEvolver(NULL, getColor(cIdx++, color)) );
				Snaxel *head = new Snaxel(0);
				SnaxelFront front;
				front.snaxelHead = head;
				Snaxel *s = head;
				for(int k=0; k < (*map_polygons)[i].contour[j].num_vertices; k++) {
					s->u[0] = float(w)*(1.f+(*map_polygons)[i].contour[j].vertex[k].x)/2.f;
					s->u[1] = float(h)*(1.f-(*map_polygons)[i].contour[j].vertex[k].y)/2.f;
					if( k == (*map_polygons)[i].contour[j].num_vertices-1 ) { //end of list
						s->right = head;
					} else {
						s->right = new Snaxel(k+1);
					}
					s->right->left = s;
					s = s->right;
				}
				(**evolsOut).back()->getSnaxelFronts()->push_back( front );
			}
		}
	}
	//Return
	return map_polygons;
}

void PlanarMapDetector::renderPlanarMap( vector<gpc_polygon>* p ) {
	if(p == NULL) return;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glLineWidth(5);
	float color[3];
	int cIdx=0;
srand( time(NULL) );

	for(unsigned i = 0; i < p->size(); i++) {
printf("Polygon %d, num contours = %d\n", i, (*p)[i].num_contours);
		for(int j = 0; j < (*p)[i].num_contours; j++) {
printf("\t Contour %d = Hole? %d\n", j, (*p)[i].hole[j]);
			glColor3fv( getColor(cIdx++, color) );
			glBegin(GL_LINE_LOOP);
			for(int k = 0; k < (*p)[i].contour[j].num_vertices; k++) {
				glVertex2f((*p)[i].contour[j].vertex[k].x, (*p)[i].contour[j].vertex[k].y);
			}
			glEnd();
		}
	}

//Draw polygons
for(unsigned i = 0; i < p->size(); i++) {
	gpc_tristrip t;
	gpc_polygon_to_tristrip( &((*p)[i]), &t);
	glColor3fv( getColor(cIdx++, color) );
	for(int j = 0; j < t.num_strips; j++) {
		glBegin(GL_TRIANGLE_STRIP);	
		for(int k = 0; k < t.strip[j].num_vertices; k++)
			glVertex2f( t.strip[j].vertex[k].x, t.strip[j].vertex[k].y );
		glEnd();
	}
}


	glPopMatrix(); //Pop modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix(); //Pop projection
}
