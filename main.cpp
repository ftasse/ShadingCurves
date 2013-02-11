#include <QApplication>
#include "Utilities/ImageUtils.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.setWindowTitle("2D Image lighting and shading");
    w.showMaximized();
    w.scene->openImage(imageLocationWithID("bartsimpson.jpg"));

    return a.exec();
    //displayImageWithID("bartsimpson.jpg");
}
