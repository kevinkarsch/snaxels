/*
 * SVGWidget.h
 * SVG display widget
 */

#ifndef SVGWIDGET_H
#define SVGWIDGET_H

#include <QtSvg>
#include <QSvgWidget>
#include "SVGCreator.h"

class SVGWidget : public QSvgWidget
{
	Q_OBJECT

public:
	SVGWidget(QWidget *parent = 0);
	~SVGWidget();
	QSize sizeHint() const { return QSize(400, 400); };
	void setAnimation(SVGCreator *a) { animation = a; };
	SVGCreator* getAnimation() { return animation; };
	QString getInitialCSS(vector<SnaxelEvolver*>* evolvers);
	void saveSWFAnimation(QString& swffile);
	void saveSVGAnimation(QString svgfile);

	void setFrame(int frameN);
	void setDefs(QString& defsbody) { animation->setDefs(defsbody); };
	void setCSS(QString& cssbody) { animation->setCSS(cssbody); };

protected:

private:
	SVGCreator *animation;
};

#endif
