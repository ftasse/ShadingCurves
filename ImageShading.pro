#-------------------------------------------------
#
# Project created by QtCreator 2013-01-31T20:42:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageShading
TEMPLATE = app

MOC_DIR = Build/moc/
OBJECTS_DIR = Build/obj/


SOURCES += main.cpp\
        mainwindow.cpp \
    Utilities/ImageUtils.cpp \
    Views/GraphicsImageItem.cpp \
    Views/GraphicsView.cpp

HEADERS  += mainwindow.h \
    Utilities/ImageUtils.h \
    Views/GraphicsImageItem.h \
    Views/GraphicsView.h

FORMS    += mainwindow.ui

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui
