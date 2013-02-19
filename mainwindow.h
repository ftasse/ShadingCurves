#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Views/GLScene.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    GLScene *scene;
    
private:
    Ui::MainWindow *ui;

public slots:
    void center();
};

#endif // MAINWINDOW_H
