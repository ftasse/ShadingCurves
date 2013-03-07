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
    void changeSurfaceWidth(int surfaceWidth);
    void changeBrushLightness(int type);
    void changeBrushSize(int size);
    void changeFreehand(bool freehand);
    void showControlMesh(bool status);
    void showControlPoints(bool status);
    void createDistanceTransformDEBUG();
    void show3Dwidget();
    void applyShading(bool showImg, bool writeImg);
    void applyShading();
    void setBrush();
    void changeBrushTypeC(bool val);
    void changeBrushTypeD(bool val);
    void setSuper1();
    void setSuper2();
    void setSuper4();
    void setImgShowAll(bool b);
    void setImgWriteAll(bool b);

private:
    DebugWindow     *dbw;
    MainWindow3D    *glw;
    GLviewsubd      *glvs;

    int superSampling;
    bool imgShowAll, imgWriteAll;
};

#endif // GRAPHICSVIEW_H
