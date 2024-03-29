######################################################################
# Automatically generated by qmake (2.01a) Sun Apr 10 20:56:25 2011
######################################################################

CONFIG += qt
QT += opengl xml svg
TEMPLATE = app
TARGET = Snaxels
DEPENDPATH += ../../Snaxels
INCLUDEPATH += . ..

# Input
HEADERS += ../ArcBall.h \
           ../common.h \
           ../glew.h \
           ../GLWidget.h \
           ../GLWindow.h \
           ../gpc.h \
           ../Mesh.h \
           ../PlanarMapDetector.h \
           ../rply.h \
           ../SnaxelCreatorWidget.h \
           ../SnaxelEvolver.h \
           ../SVGCreator.h \
           ../SVGWidget.h \
           ../timer.h
FORMS += ../GLWindow.ui ../SnaxelCreatorWidget.ui
SOURCES += ../ArcBall.cpp \
           ../glew.c \
           ../GLWidget.cpp \
           ../GLWindow.cpp \
           ../gpc.c \
           ../main.cpp \
           ../Mesh.cpp \
           ../PlanarMapDetector.cpp \
           ../rply.c \
           ../SnaxelCreatorWidget.cpp \
           ../SnaxelEvolver.cpp \
           ../SVGCreator.cpp \
           ../SVGWidget.cpp
