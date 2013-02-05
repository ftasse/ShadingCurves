#-------------------------------------------------
#
# Project created by QtCreator 2013-01-31T20:42:25
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageShading
TEMPLATE = app

MOC_DIR = Build/moc/
OBJECTS_DIR = Build/obj/


SOURCES += main.cpp\
        mainwindow.cpp \
    Utilities/ImageUtils.cpp \
    Views/GraphicsView.cpp \
    Views/GLScene.cpp \
    Curve/ControlPoint.cpp \
    Curve/BSpline.cpp

HEADERS  += mainwindow.h \
    Utilities/ImageUtils.h \
    Views/GraphicsView.h \
    Views/GLScene.h \
    Curve/ControlPoint.h \
    Curve/BSpline.h

FORMS    += mainwindow.ui

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lGLU
