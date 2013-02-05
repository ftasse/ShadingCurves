#include <QApplication>
#include "Utilities/ImageUtils.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("2D Image lighting and shading");
    //w.loadImage(imageLocationWithID("clouds3.jpg"));

    w.show();
    return a.exec();
}
