#include <QApplication>

#include "GLWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
	
	//Enable multisampling
	QGLFormat fmt;
    fmt.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(fmt);
	
    GLWindow glWindow;
    glWindow.show();
    return app.exec();
}