#include "SVGWidget.h"

SVGWidget::SVGWidget(QWidget *parent) : QSvgWidget(parent) {
	animation = NULL;
}

SVGWidget::~SVGWidget() {
}

QString SVGWidget::getInitialCSS(vector<SnaxelEvolver*>* evolvers) {
	return animation->getInitialCSS(evolvers);
}

void SVGWidget::setFrame(int frameN) {
	if(!animation || animation->nFrames()<1) return;
	if(frameN < 0) frameN = 0;
	if(frameN >= animation->nFrames()) frameN = animation->nFrames()-1;
	load( animation->svgFileAsString(frameN).toUtf8() );
	animation->saveSVGFile("debug.svg", frameN);
}

//This is a windows only function!!
void SVGWidget::saveSWFAnimation(QString& swffile) {
#ifdef WIN32
	system("mkdir .tmpsvg .tmpswf");
	cout << "Compiling to: " << swffile.toStdString() << "...";
	vector<QString> compilecmds;
	int fps = 10;
	for(int i=0; i<animation->nFrames(); i++) {
		if(i%100==0) {
			compilecmds.push_back( QString("swfcombine --cat --zlib --rate %1 -o .tmpswf/seg%2.swf ").arg(fps).arg(compilecmds.size()) );
			for(int j=0; j<compilecmds.size()-1; j++)
				compilecmds.back() += QString(".tmpswf/seg%1.swf ").arg(j);
		}
		QString svg = QString(".tmpsvg/%1.svg").arg(i,5,10,QChar('0'));
		QString swf = QString(".tmpswf/%1.swf").arg(i,5,10,QChar('0'));
		animation->saveSVGFile(svg, i);
		system( QString("svg2swf --cubic 5 --asf pan_zoom.as  %1 %2").arg(svg).arg(swf).toAscii() );
		compilecmds.back() += swf + QString(" ");
		cout << ".";
	}
	for(int i=0; i<compilecmds.size(); i++) {
		system( compilecmds[i].toAscii() );
		cout << ".";
	}
	QString movestr = QString("move /y .tmpswf\\seg%1.swf %2 > NUL").arg(compilecmds.size()-1).arg(swffile.replace("/","\\"));
	system( movestr.toAscii() );
	system("rmdir .tmpsvg .tmpswf /s /q");
	cout << "\nDone!\n\n";
#else
	cout << "Method not implemented for platforms other than win32!!" << endl;
#endif
}

void SVGWidget::saveSVGAnimation(QString svgfile) {
	animation->saveSVGAnimation(svgfile);
}