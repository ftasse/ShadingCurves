#-------------------------------------------------
#
# Project created by QtCreator 2013-01-31T20:42:25
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageShading
TEMPLATE = app

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

win32 {
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
}

INCLUDEPATH += $$PWD
win32:INCLUDEPATH += C:/opencv/build/include

win32:LIBS +=  -L"C:/opencv-build/bin" \
               -L"C:/opencv/build/x86/vc10/bin" \
               -L"C:/Qt/Qt5.0.1/Tools/MinGW/bin"
win32:LIBS += -lopencv_core243 -lopencv_highgui243 -lopengl32
unix:LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lGLU

