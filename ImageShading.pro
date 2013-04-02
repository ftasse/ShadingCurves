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
    Curve/ControlPoint.cpp \
    Curve/Surface.cpp \
    Views/DebugWindow.cpp \
    3D/control.cpp \
    3D/glabstract.cpp \
    3D/glbar.cpp \
    3D/glviewport.cpp \
    3D/glviewsubd.cpp \
    3D/mainwindow3d.cpp \
    3D/mesh.cpp \
    3D/point_3d.cpp \
    Views/glew/glew.c \
    Utilities/SurfaceUtils.cpp \
    Curve/Point3d.cpp \
    Views/ShadingProfileView.cpp

HEADERS  += mainwindow.h \
    Utilities/ImageUtils.h \
    Views/GraphicsView.h \
    Views/GLScene.h \
    Curve/BSpline.h \
    Curve/BSplineGroup.h \
    Curve/ControlPoint.h \
    Curve/Surface.h \
    Views/DebugWindow.h \
    3D/control.h \
    3D/glabstract.h \
    3D/glbar.h \
    3D/glviewport.h \
    3D/glviewsubd.h \
    3D/mainwindow3d.h \
    3D/mesh.h \
    3D/point_3d.h \
    3D/tostring.h \
    3D/types.h \
    Views/glew/glew.h \
    Views/glew/GL/wglew.h \
    Views/glew/GL/glxew.h \
    Views/glew/GL/glew.h \
    Utilities/SurfaceUtils.h \
    Curve/Point3d.h \
    Views/ShadingProfileView.h

FORMS    += mainwindow.ui \
    Views/DebugWindow.ui

win32 {
QMAKE_LFLAGS += -static-libgcc -static-libstdc++
DEFINES += WIN32 _WIN32
DEFINES += USING_GLEW
DEFINES += GLEW_STATIC
}

INCLUDEPATH += $$PWD
win32:INCLUDEPATH += C:/opencv/build/include
#unix:INCLUDEPATH += /local/scratch/jk520/OpenCV-2.4.4/include

win32:LIBS +=  -L"C:/opencv-build/bin" \
               -L"C:/opencv-build/lib" \
               -L"C:/opencv/build/x86/vc10/bin"

win32:LIBS += -lopencv_core243 -lopencv_highgui243 -lopencv_imgproc243 -lopengl32 -lglu32
unix:LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lGLU

unix:QMAKE_CXXFLAGS += -std=c++0x
unix:QMAKE_CFLAGS += -std=c++0x

unix:LIBS += -fopenmp
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_RELEASE += -fopenmp
QMAKE_CFLAGS_RELEASE += -fopenmp
