#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include "../Views/DebugWindow.h"
#include "../3D/mainwindow3d.h"

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent);

protected:
    void resizeEvent(QResizeEvent *event);
    
signals:
    
public slots:
    void create_bspline();
    void move_bsplines();
    void edit_bspline();
    void loadImage();
    void saveImage();
    void loadCurves();
    void saveCurves();

    // HENRIK: add write to off-file support
    void saveOff();

    void changeControlPointSize(int pointSize);
    void showControlMesh(bool status);
    void showControlPoints(bool status);
    void createDistanceTransformDEBUG();
    void show3Dwidget();

private:
    DebugWindow     *dbw;
    MainWindow3D    *glw;
};

#endif // GRAPHICSVIEW_H
