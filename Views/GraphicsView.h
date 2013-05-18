#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include "../Views/DebugWindow.h"
#include "../3D/mainwindow3d.h"

//enum ShadingType
//{
//    CVLAB, CVHLS, OWN
//};

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent);

    //void setCenter(const QPointF& centerPoint);

protected:
    void resizeEvent(QResizeEvent *event);
    //void wheelEvent(QWheelEvent* event);
    
signals:
    void setStatusMessage(QString message);
    void setTimeOutput(QString t);
    void setTimeOutputSub(QString t);
    void setMultiSubdOutputMin(QString t);
    void setMultiSubdOutputAvrg(QString t);
    void setMultiSubdOutputMax(QString t);
    
public slots:
    void create_bspline();
    void move_bsplines();
    void edit_bspline();
    void loadImage();
    void saveImage();
    void saveScreenshot();
    void saveCurrentState();
    void loadBackgroungImage();
    void loadCurves();
    void saveCurves();
    void loadProject();
    void saveProject();
    void backupProject();

    // HENRIK: add write to off-file support
    void saveOff();

    void toggleBackupStatus(bool b);
    void changeControlPointSize(int pointSize);
    void changeBrushLightness(int type);
    void changeBrushSize(int size);
    void changeFreehand(bool freehand);
    void changeResolution();
    void changeCurveSubdLevels(int value);
    void changeDisplaySubdLevels(int value);
    void changeGlobalThickness(int value);
    void showControlMesh(bool status);
    void showControlPoints(bool status);
    void showCurves(bool status);
    void showNormals(bool status);
    void showColors(bool status);
    void createDistanceTransformDEBUG();
    void showDistanceTransform3D();
    void showCurvesImage3D();
    void show3Dwidget();
    void applyShading(bool showImg, bool writeImg);
    void applyShading();
    void setBrush();
    void changeBrushTypeC(bool val);
    void changeBrushTypeD(bool val);
    void setSuper1();
    void setSuper2();
    void setSuper4();
    void setSurfSubdLevel(int);
    void setImgShowAll(bool b);
    void setImgWriteAll(bool b);
    void setClipping(bool b);
    void setClipMin(int min);
    void setClipMax(int max);
    void setShadingLab();
    void setShadingHLS();
    void setShadingOwn();
    void setShadingMatlab();
    void setShadingYxy();
    void setShadingRGB();
    void setBlackOut(bool b);
    void setFlatImage(bool b);
    void setClrVsTxtr(bool b);
    void setMultiSubd(int m);
    void runMultiSubd();

private:
    QPointF currentCenterPoint;

    DebugWindow     *dbw;
    MainWindow3D    *glw;
    GLviewsubd      *glvs;

    int superSampling, surfSubdLevel, clipMin, clipMax, subdivTime, multiSubd;
    bool imgShowAll, imgWriteAll, clipping, timeIt, blackOut, interactiveShading, flatImage, clrVsTxtr;

    ShadingType     shade;
    QString         pathToData;
    QTimer *backupTimer;
    QString backupName;
};

#endif // GRAPHICSVIEW_H
