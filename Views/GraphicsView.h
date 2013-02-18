#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include "../Views/DebugWindow.h"

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

private:
    DebugWindow *dbw;
    
};

#endif // GRAPHICSVIEW_H
