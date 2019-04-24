#include <QtGui>

#include "SnaxelCreatorWidget.h"

SnaxelCreatorWidget::SnaxelCreatorWidget( GLWindow *parent ) {
    setupUi(this); // this sets up GUI
	
	QObject::connect(goalNVSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateNVSlider()));
	QObject::connect(goalNVSlider, SIGNAL(valueChanged(int)), this, SLOT(updateNVSpinBox()));
	QObject::connect(createButton, SIGNAL(clicked()), this, SLOT(createSnaxels()));
}

void SnaxelCreatorWidget::updateNVSpinBox() {
	goalNVSpinBox->setValue( double(goalNVSlider->value()) / 100.f );
}

void SnaxelCreatorWidget::updateNVSlider() {
	goalNVSlider->setValue( (int)(goalNVSpinBox->value() * 100.f) );
}

void SnaxelCreatorWidget::createSnaxels() {
	this->accept();
}

void SnaxelCreatorWidget::getParameters(float *nvmin) {
	*nvmin = float( goalNVSpinBox->value() );
}
