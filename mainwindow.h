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
    void change_bspline_parameters();
    void update_bspline_parameters_ui(bool enabled, float extent, bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward);
};

#endif // MAINWINDOW_H
