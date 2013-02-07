#-------------------------------------------------
#
# Project created by QtCreator 2013-01-31T20:42:25
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageShading
TEMPLATE = app

MOC_DIR = ../imageshading-build/moc/
OBJECTS_DIR = ../imageshading-build/obj/


SOURCES += main.cpp\
        mainwindow.cpp \
    Utilities/ImageUtils.cpp \
    Views/GraphicsView.cpp \
    Views/GLScene.cpp \
    Curve/BSpline.cpp \
    Curve/BSplineGroup.cpp \
    Curve/ControlPoint.cpp

HEADERS  += mainwindow.h \
    Utilities/ImageUtils.h \
    Views/GraphicsView.h \
    Views/GLScene.h \
    Curve/BSpline.h \
    Curve/BSplineGroup.h \
    Curve/ControlPoint.h

FORMS    += mainwindow.ui

INCLUDEPATH += ./

unix:LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lGLU

win32:CONFIG(release, debug|release): LIBS += -L./Libs/opencv/lib/ -L./Libs/opencv/bin/  -lopencv_core243 -lopencv_imgproc243 -lopencv_highgui243
else:win32:CONFIG(debug, debug|release): LIBS += -L./Libs/opencv/lib/ -lopencv_core243d -lopencv_imgproc243d -lopencv_highgui243d

win32:INCLUDEPATH += ./Libs/opencv/include
win32:DEPENDPATH += ./Libs/opencv/include
