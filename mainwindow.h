#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "Views/GraphicsImageItem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QGraphicsScene * scene;
    GraphicsImageItem *imageItem;

    void setupImageView();

public slots:
    void loadImage();
    void loadImage(std::string imageLocation);
};

#endif // MAINWINDOW_H
