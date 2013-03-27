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

    void showStatusMessage(QString message);

    void change_point_sharpness();
    void update_point_sharpness_ui(bool enabled, bool isSharp);

    void change_bspline_parameters();
    void change_slope_curve();
    void change_inward_outward_direction();
    void change_uniform_subdivision();
    void update_bspline_parameters_ui(bool enabled, float extent, bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward,  int thickness);
};

#endif // MAINWINDOW_H
