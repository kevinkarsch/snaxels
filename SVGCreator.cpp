#include "SVGCreator.h"


float intrvl = 0.05f;
float off_intrvl = 0.035f;
float sizecurve = 14.f;
float distdir=0.f;

SVGCreator::SVGCreator(int w, int h) {
	svgHeader = "<?xml version=\"1.0\" standalone=\"no\"?>\n";
	svgHeader += "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n";
	svgHeader += "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
	svgHeader += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"\n";
	svgHeader += "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
	svgHeader += QString("width=\"%1\" height=\"%2\" viewBox=\"0 0 %1 %2\">\n\n").arg(w).arg(h);
	svgFooter = "</svg>\n";
	width = w; height = h;
}

SVGCreator::~SVGCreator() {
}

inline void proj3Dpoint(float *p3d, float *proj, float w, float h, float *result) {
	float norm = proj[3]*p3d[0] + proj[7]*p3d[1] + proj[11]*p3d[2] + proj[15];
	result[0] = (proj[0]*p3d[0] + proj[4]*p3d[1] + proj[8]*p3d[2] + proj[12]) / norm;
	result[0] = 0.5f*(result[0]+1.f)*w;
	result[1] = (proj[1]*p3d[0] + proj[5]*p3d[1] + proj[9]*p3d[2] + proj[13]) / norm;
	result[1]= 0.5f*(-result[1]+1.f)*h; //- to flip y orientation
}


//////////////////////////////////////////////////////////////////////
//////////////////////Static SVG frame////////////////////////////////
//////////////////////////////////////////////////////////////////////
void SVGCreator::recordStaticFrame( vector<SnaxelEvolver*>* evolvers, float *proj, float w, float h ) {
	vector<SVGContour> subframes;
	for(int k=0; k<evolvers->size(); k++) {
		SVGContour contour( QString("%1").arg(k, 2, 10, QChar('0')) );
		vector<SnaxelFront>* fronts = (*evolvers)[k]->getSnaxelFronts();
		for(unsigned i = 0; i < (*fronts).size(); i++) {
			contour.hole.push_back( (*fronts)[i].holeContour );
			contour.fill.push_back( vector< pair<float,float> >() );
			contour.stroke.push_back( vector< pair<float,float> >() );
			contour.marker.push_back( vector< pair<float,float> >() );
			contour.centroid.push_back( pair<float,float>() );
float curpos = 0.f;
bool make_line = true;
			int wraparound = 0; //Makes sure first and last vertex are connected
			Snaxel *s = (*fronts)[i].snaxelHead;
			do {
				float sf = 0.9f; //Laplacian smoothing factor in [0,1] (larger=>smoother)
				sf /= 2.f;
				float su_x = s->left->u[0]*sf + s->u[0]*(1.f-2.f*sf) + s->right->u[0]*sf;
				float su_y = s->left->u[1]*sf + s->u[1]*(1.f-2.f*sf) + s->right->u[1]*sf;
				//Fill				
				contour.fill.back().push_back( make_pair(su_x, su_y) );
				contour.centroid.back().first += su_x;
				contour.centroid.back().second += su_y;
				//Stroke only where not occluded (break up into contiguous strokes)
				if( !s->occluded ) {
					contour.stroke.back().push_back( make_pair(su_x, su_y) );
				} else if( s->occluded && !s->right->occluded ) {
					contour.stroke.push_back( vector< pair<float,float> >() );
				}
//Silhouttes
/*
float su_[2];
{
	float p1[3] = {s->p[0],s->p[1], s->p[2]}; 
	float p2[3] = {s->right->p[0],s->right->p[1], s->right->p[2]};
	//if( !s->occluded && !s->right->occluded ) {
		if(make_line) {
			proj3Dpoint(p1, proj, w, h, su_);
			contour.marker.back().push_back( make_pair(su_[0], su_[1] ) );
		}
	//}
	float dist = dist3(p1, p2);
	float dir[3] = {(p2[0]-p1[0])/dist, (p2[1]-p1[1])/dist, (p2[2]-p1[2])/dist};
	float t = curpos;
	while(dist-t > 0.f ) {
		float p_i[3] = {p1[0]+t*dir[0], p1[1]+t*dir[1], p1[2]+t*dir[2]};
		//if( !s->occluded && !s->right->occluded ) {
			proj3Dpoint(p_i, proj, w, h, su_);
			contour.marker.back().push_back( make_pair(su_[0], su_[1] ) );
		//}
		if(make_line) {
			t += off_intrvl;
			contour.marker.push_back( vector< pair<float,float> >() );
			make_line = false;
		} else {
			t += intrvl;
			make_line = true;
		}
	}
	curpos = t-dist;
}	
*/
				/*//Marker (densely sample contours) -- need to fix this
				if( !s->occluded && !s->right->occluded ) {
					float p1[2] = {s->u[0],s->u[1]}, p2[2] = {s->right->u[0],s->right->u[1]};
					float dist = dist2(p1, p2);
					if(dist>1.0) {
						float dir[2] = {(p2[0]-p1[0])/dist, (p2[1]-p1[1])/dist};
						float t = dist / floor(dist);
						for(int j=0; j<int(floor(dist)); j++)
							contour.marker.back().push_back( 
								make_pair(p1[0]+float(t)*dir[0], p1[1]+float(t)*dir[1]) );
					}
				} else if( s->occluded && !s->right->occluded && !s->right->right->occluded ) {
					contour.marker.push_back( vector< pair<float,float> >() );
				}*/
				//Goto next snaxel
				s = s->right;
			} while( s!=(*fronts)[i].snaxelHead->right || wraparound++<=0 );
			contour.centroid.back().first /= contour.fill.back().size();
			contour.centroid.back().second /= contour.fill.back().size();
//if( !s->occluded && !s->right->occluded ) { //Silhouettes
	if(make_line) {
		float su_[2];
		proj3Dpoint(s->p, proj, w, h, su_);
		contour.marker.back().push_back( make_pair(su_[0], su_[1] ) );
	}
//}
		}
		subframes.push_back(contour);
	}
	frames.push_back(subframes);
}

void SVGCreator::saveSVGFile(const QString& fileName, int frameN) {
	ofstream myfile(fileName.toAscii());
	QString svgfile = svgFileAsString(frameN);
	myfile << std::string(svgfile.toAscii().constData(), svgfile.toAscii().length());
	myfile.close();
}

QString SVGCreator::svgFileAsString(int frameN) {
	return svgHeader + svgDefs + svgCSS + svgBody(frameN) + svgFooter;
}

void SVGCreator::setDefs(QString& defsbody) {
	svgDefs = QString("<defs>\n")+defsbody+QString("\n</defs>\n\n");
}

void SVGCreator::setCSS(QString& cssbody) {
	svgCSS = QString("<style type=\"text/css\" > <![CDATA[\n") + 
				cssbody + QString("\n]]> </style>\n\n");
}

QString SVGCreator::getInitialCSS(vector<SnaxelEvolver*>* evolvers) {
	QString initcss;
	for(int i=0; i<evolvers->size(); i++) {
		float* fillcol = (*evolvers)[i]->getColor();
		QString id = QString("%1").arg(i, 2, 10, QChar('0'));
		initcss += QString("path.fill_")+id+QString(" {\n\tfill: rgb(%1,%2,%3);\n\t"
			"fill-opacity: 1;\n\tstroke: none;\n\tstroke-width: 0;\n\t"
			"stroke-opacity: 1;\n\tstroke-linecap: round;\n\tstroke-linejoin: round;\n}"
			"\n").arg(int(floor(fillcol[0]*255.0))).arg(int(floor(fillcol[1]*255.0))).arg(int(floor(fillcol[2]*255.0)));
		initcss += QString("path.stroke_")+id+QString(" {\n\tfill: none;\n\t"
			"fill-opacity: 1;\n\tstroke: rgb(0,0,0);\n\tstroke-width: 3;\n\t"
			"stroke-opacity: 1;\n\tstroke-linecap: round;\n\tstroke-linejoin: round;\n}\n");
		initcss += QString("path.marker_")+id+QString(" {\n\tfill: none;\n\t"
			"stroke: none;\n\tmarker-start: url(#marker);\n\t"
			"marker-mid: url(#marker);\n\tmarker-end: url(#marker);\n}\n");
		initcss += "\n";
	}
	return initcss;
}

QString SVGCreator::svgBody(int frameN, bool noFill) {
	vector<SVGContour>* frame = &frames[frameN];
	QString body;
	

if(!noFill) {
	for(int i=0; i<frame->size(); i++) {
		//First write hole contours (as svg mask)
		QString maskid = QString("mask_")+(*frame)[i].id;
		body += svgMasksFromContour( (*frame)[i], maskid );
		//Then all other fills
		for(int j=0; j<(*frame)[i].fill.size(); j++)
			if( !(*frame)[i].hole[j] )
				body += svgPath( (*frame)[i].fill[j], &(*frame)[i].centroid[j], QString("\"fill_")+(*frame)[i].id+QString("\"") );//, maskid );
	}
}
	for(int i=0; i<frame->size(); i++) {
		//Strokes
		for(int j=0; j<(*frame)[i].stroke.size(); j++)
			body += svgPath( (*frame)[i].stroke[j], NULL, QString("\"stroke_")+(*frame)[i].id+QString("\"") );
	}
	/* //For kalnin's silhouettes
	for(int i=0; i<frame->size(); i++) {
		//Markers
		for(int j=0; j<(*frame)[i].marker.size(); j++)
			if( (*frame)[i].marker[j].size() >= 2 )
				body += svgPath( (*frame)[i].marker[j], NULL, QString("\"stroke_")+(*frame)[i].id+QString("\"") );
		for(int j=0; j<(*frame)[i].marker.size(); j++) {
			if( (*frame)[i].marker[j].size() < 2 ) continue;
			float p0[2] = { (*frame)[i].marker[j].front().first, (*frame)[i].marker[j].front().second };
			float p2[2] = { (*frame)[i].marker[j].back().first, (*frame)[i].marker[j].back().second };
			if( dist2(p0,p2)<EPSILON ) continue;
			float midp[2] = { 0.5f*(p0[0]+p2[0]), 0.5f*(p0[1]+p2[1]) };
			float dir[3] = {p2[0]-p0[0], p2[1]-p0[1], 0};
			normalize(dir);
			float midn[3] = { -dir[1], dir[0], 0 };
			normalize(midn);
			float p1[2] = { midp[0]+midn[0]*sizecurve, midp[1]+midn[1]*sizecurve };
			p0[0] -= distdir*dir[0]; p0[1] -= distdir*dir[1];
			p2[0] += distdir*dir[0]; p2[1] += distdir*dir[1];
			//Or let svg do the work
			//body += QString("<path class=%1 d=\"M%2,%3 Q%4,%5 %6,%7\"/>\n").arg(QString("\"stroke_")+(*frame)[i].id+QString("\"")).arg(
			//	p0[0]).arg(p0[1]).arg(p1[0]).arg(p1[1]).arg(p2[0]).arg(p2[1]);
		}
	}
	*/
	return body+QString("\n");
}

QString SVGCreator::svgPath( vector< pair<float,float> >& p, pair<float,float> *centroid, QString cid, QString maskid ) {
	if(p.size()<1) return QString();
	QString path;
	if( centroid!=NULL ) 
		path += QString("<g transform=\"translate(%1 %2) scale(1) translate(-%1 -%2)\">\n").arg(centroid->first).arg(centroid->second);
	if(maskid==QString("")) path += QString("<path class=")+cid;
	else path += QString("<path mask=\"url(#")+maskid+QString(")\" class=")+cid;
	path += QString(" d=\"M%1,%2 ").arg(p[0].first).arg(p[0].second);
	for(int i=1; i <p.size(); i++)
		path += QString("L%1,%2 ").arg(p[i].first).arg(p[i].second);
	path += QString("\"/>\n");
	if( centroid!=NULL ) path += QString("</g>\n");
	return path;
}

QString SVGCreator::svgMasksFromContour( SVGContour& c, QString maskid ) {
	QString maskpath = QString("<mask id=\"")+maskid+QString("\">\n");
	maskpath += QString("<path style=\"fill:white\" d=\"M0,0 L%1,0 L%2,%3, L0,%4\"/>\n").arg(width).arg(width).arg(height).arg(height);
	for(int i=0; i < c.fill.size(); i++) {
		if(!c.hole[i] || c.fill[i].size()<1) continue;
		maskpath += QString("<path style=\"fill:black\"");
		maskpath += QString(" d=\"M%1,%2 ").arg(c.fill[i][0].first).arg(c.fill[i][0].second);
		for(int j=1; j <c.fill[i].size(); j++)
			maskpath += QString("L%1,%2 ").arg(c.fill[i][j].first).arg(c.fill[i][j].second);
		maskpath += QString("\"/>\n");
	}
	return maskpath+QString("</mask>\n");
}


//////////////////////////////////////////////////////////////////////
//////////////////////Dynamic SVG Animation///////////////////////////
//////////////////////////////////////////////////////////////////////
void SVGCreator::recordDynamicFrame( vector<SnaxelEvolver*>* evolvers, Mesh* mesh ) {
	//Check if we need to start a new clip
	int clip = dEvolvers.size()-1;
	bool newclip = (clip<0); //automatically start new clip if clip==-1, otherwise check below conditions
	int frame;
	if(clip >= 0 && dEvolvers[clip].size()-1 >= 0) {
		frame = dEvolvers[clip].size()-1;
		if( (*evolvers).size() != dEvolvers[clip][frame].size() ) //Has a new evolver been added?
			newclip = true;
		for(int i=0; i<(*evolvers).size(); i++) { //New fronts created (visual event)?
			if((*evolvers)[i]->getSnaxelFronts()->size() != dEvolvers[clip][frame][i]->getSnaxelFronts()->size()) {
				newclip = true;
				break;
			}
		}
	}
	if(newclip) {
		dEvolvers.push_back( vector< vector<SnaxelEvolver*> >() ); //If true, push back a new clip
		clip = dEvolvers.size()-1; //One cip for every visual event/change in topology
	}
	// Push back a new frame
	dEvolvers[clip].push_back( vector<SnaxelEvolver*>() );
	frame = dEvolvers[clip].size()-1; //A given frame of a clip
	for(unsigned i = 0; i < (*evolvers).size(); i++) { //Copy snaxel data to the frame
		dEvolvers[clip][frame].push_back(new SnaxelEvolver(mesh));
		(*evolvers)[i]->initializeParents( dEvolvers[clip][frame][i] ); //Stores a copy of the current fronts
	}
}

void SVGCreator::eraseDynamicFrames() {
	for(int i=0; i<dEvolvers.size(); i++)
		for(int j=0; j<dEvolvers[i].size(); j++)
			for(int k = 0; k < dEvolvers[i][j].size(); k++)
				delete dEvolvers[i][j][k];
	dEvolvers.clear();
}

void SVGCreator::prepareAnimationCoherence() {
	/*
	int nclips = dEvolvers.size();
	for(int i=0; i<nclips; i++) {
		int nframes = dEvolvers[i].size();
		int nevolvers = dEvolvers[i][0].size(); //Record the number of evolvers (doesn't change within a clip)
		//For every evolver and every front, find correspondences over frames
		for(int j=0; j<nevolvers; j++) {
			int nfronts = dEvolvers[i][0][j]->getSnaxelFronts()->size();
			for(int k=0; k<nfronts; k++) {
				int Midx, M=0;
				for(int l=0; l<nframes; l++) { //Find frame with most snaxels
					int sz = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].calcSize();
					if(sz>M) { M = sz; Midx = l; }
				}
				//Backward propogation
				for(int l=Midx-1; l>=0; l--) {
					int Lsize = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].calcSize();
					int Rsize = (*dEvolvers[i][l+1][j]->getSnaxelFronts())[k].calcSize();
					if(Rsize==Lsize) continue;
					Snaxel* Rhead = (*dEvolvers[i][l+1][j]->getSnaxelFronts())[k].snaxelHead;
					//Fix parent pointers so there are no duplicates
					Snaxel* s = Rhead;
					do {
						Snaxel* stmp = s;
						while(stmp->right->parent == s->parent) {
							stmp->right->parent = s->parent->right;
							stmp = stmp->right;
						}
						s = s->right;
					} while( s != Rhead );
					//clear touched bool
					s = Rhead;
					do { s->parent->touched = false; s = s->right; } 
					while(s != Rhead);
					//Propogate
					s = Rhead;
					do {
						Snaxel *par = s->parent;
						if(!par->touched) 
							par->touched = true;
						else { //Add a new snaxel
							Snaxel *par_add = new Snaxel(par);
							par_add->left = par;
							par_add->right = par->right;
							par->right->left = par_add;
							par->right = par_add;
						}
						s = s->right;
					} while(s != Rhead);
				}
				//Forward propogation
				for(int l=Midx+1; l<nframes; l++) {
					int Lsize = (*dEvolvers[i][l-1][j]->getSnaxelFronts())[k].calcSize();
					int Rsize = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].calcSize();
					if(Rsize==Lsize) continue;
					Snaxel* Rhead = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].snaxelHead;
					//Fix parent pointers so there are no duplicates
					Snaxel* s = Rhead;
					bool keepGoing;
					do {
						keepGoing = false;
						do {
							Snaxel* stmp = s;
							while(stmp->right->parent == s->parent) {
								stmp->right->parent = s->parent->right;
								stmp = stmp->right;
								keepGoing = true;
							}
							s = s->right;
						} while( s != Rhead );
					} while(keepGoing);
					//Add new snaxels
					s = Rhead;
					int idx=0;
					do {
						Snaxel *par1 = s->parent;
						Snaxel *par2 = s->right->parent;
						int numBetween = 0;
						if(par1!=par2) { //Check if any new snaxels should be inserted
							while(par1->right != par2) {
								par1 = par1->right;
								numBetween++;
							}
							for(int a=0; a<numBetween; a++) {
								Snaxel *s_add = new Snaxel(s);
								s_add->left = s;
								s_add->right = s->right;
								s->right->left = s_add;
								s->right = s_add;
								s = s->right;
							}
						} 
						s = s->right;
					} while(s != Rhead);
				}
				//Set heads properly
				int finalSize = (*dEvolvers[i][nframes-1][j]->getSnaxelFronts())[k].snaxelHead->calcSize();
				for(int l=nframes-1; l>0; l--) { 
					(*dEvolvers[i][l-1][j]->getSnaxelFronts())[k].snaxelHead = 
						(*dEvolvers[i][l][j]->getSnaxelFronts())[k].snaxelHead->parent;
					int sz = (*dEvolvers[i][l-1][j]->getSnaxelFronts())[k].snaxelHead->calcSize();
					if(finalSize<sz) finalSize = sz;
				}
				//Fix small errors (coding mistakes?); insert to satisfy all frames
				for(int l=0; l<nframes; l++) {
					Snaxel *frontHead = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].snaxelHead;
					int diff = finalSize - frontHead->calcSize();
					for(int a=0; a<diff; a++) {
						Snaxel *frontHead_add = new Snaxel(frontHead);
						frontHead_add->left = frontHead;
						frontHead_add->right = frontHead->right;
						frontHead->right->left = frontHead_add;
						frontHead->right = frontHead_add;
					}
				}

//debug print statements
				printf("size (%d,%d,%d) -- finalSize = %d\n", i,j,k, finalSize);
				for(int l=0; l<nframes; l++) { 
					printf("\t=%d\n", (*dEvolvers[i][l][j]->getSnaxelFronts())[k].snaxelHead->calcSize());
				}
			}
		}
	}

	//////////////////// testing
	for(int i=0; i<nclips; i++) {
		clearFrames();
		int nframes = dEvolvers[i].size();
		for(int j=0; j<nframes; j++) {
			recordStaticFrame(&(dEvolvers[i][j]));
			saveSVGFile(QString("C:/Users/kevin/Desktop/svg/%1/%2.svg").arg(i).arg(j), j);
		}
	}
	*/
	
}

void SVGCreator::saveSVGAnimation(const QString& fileName) {
	QString svgAnimBody;
	float curTime = 0.f, curIdx=0;
	float rate = 1.f/30.f; //fps
	int nclips = dEvolvers.size();
	for(int i=0; i<nclips; i++) {
		int nframes = dEvolvers[i].size();
		int nevolvers = dEvolvers[i][0].size();
		float duration = float(nframes)*rate;
		for(int j=0; j<nevolvers; j++) {
			int nfronts = dEvolvers[i][0][j]->getSnaxelFronts()->size();
			//Fill
			for(int k=0; k<nfronts; k++) {
				svgAnimBody += QString("<path class=\"fill_%1\">\n").arg(j, 2, 10, QChar('0'));
				svgAnimBody += QString("\t<animate attributeName=\"d\" attributeType=\"XML\" "
									   "begin=\"%1s\" dur=\"%2s\" values=\"").arg(curTime).arg(duration);
				for(int l=0; l<nframes; l++) {
					Snaxel *head = (*dEvolvers[i][l][j]->getSnaxelFronts())[k].snaxelHead;
					Snaxel *s = head;
					svgAnimBody += QString("M");
					int idx = 0;
					do {
						float sf = 0.9f; //Laplacian smoothing factor in [0,1] (larger=>smoother)
						sf /= 2.f;
						float su_x = s->left->u[0]*sf + s->u[0]*(1.f-2.f*sf) + s->right->u[0]*sf;
						float su_y = s->left->u[1]*sf + s->u[1]*(1.f-2.f*sf) + s->right->u[1]*sf;
						svgAnimBody += QString("%1,%2 ").arg(su_x).arg(su_y);
						s = s->right;
						idx++;
					} while( s != head );
					svgAnimBody += QString("z; ");
				}
				svgAnimBody += QString("\"/>\n");
				//Hide frame unless last clip&frame
				if(i<nclips-1 || j<nframes-1)
					svgAnimBody += QString("\t<set attributeName=\"visibility\" attributeType=\"XML\" "
										   "begin=\"%1s\" to=\"hidden\"/>\n").arg(curTime+duration);
				svgAnimBody += QString("</path>\n\n");

//Add strokes?		/////////////////		
for(int l=0; l<nframes; l++) {
	svgAnimBody += QString("<g visibility=\"hidden\">\n");
	svgAnimBody += QString("<set attributeName=\"visibility\" attributeType=\"XML\" "
						   "begin=\"%1s\" to=\"visible\"/>\n").arg(curTime+float(l)*rate);
	svgAnimBody += svgBody(l+curIdx, true);
	if(l+curIdx<nFrames()-1)
		svgAnimBody += QString("<set attributeName=\"visibility\" attributeType=\"XML\" "
							   "begin=\"%1s\" to=\"hidden\"/>\n").arg(curTime+float(l+1)*rate);
	svgAnimBody += QString("</g>\n\n");
}
////////////////////
			}
		}
		curIdx += nframes;
		curTime += duration;
	}	
	QString fullAnimation = svgHeader + svgDefs + svgCSS + svgAnimBody + svgFooter;
	ofstream myfile(fileName.toAscii());
	myfile << fullAnimation.toStdString();
	myfile.close();
}

void SVGCreator::saveSVGAnimationFB(const QString& fileName) {
	QString svgAnimBody;
	float rate = 1.f/15.f; //fps
	for(int i=0; i < nFrames(); i++) {
		svgAnimBody += QString("<g visibility=\"hidden\">\n");
		svgAnimBody += QString("<set attributeName=\"visibility\" attributeType=\"XML\" "
							   "begin=\"%1s\" to=\"visible\"/>\n").arg(float(i)*rate);
		svgAnimBody += svgBody(i);
		if(i<nFrames()-1)
			svgAnimBody += QString("<set attributeName=\"visibility\" attributeType=\"XML\" "
								   "begin=\"%1s\" to=\"hidden\"/>\n").arg(float(i+1)*rate);
		svgAnimBody += QString("</g>\n\n");
	}
	QString fullAnimation = svgHeader + svgDefs + svgCSS + svgAnimBody + svgFooter;
	ofstream myfile(fileName.toAscii());
	myfile << fullAnimation.toStdString();
	myfile.close();
}