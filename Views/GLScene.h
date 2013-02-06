#ifndef GLSCENE_H
#define GLSCENE_H

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include "Utilities/ImageUtils.h"
#include "Curve/BSplineGroup.h"

class ControlPointItem;
class SplinePathItem;

class GLScene : public QGraphicsScene
{
    Q_OBJECT
public:
    typedef enum SketchMode
    {
        ADD_CURVE_MODE,
        IDLE_MODE
    } SketchMode;

    explicit GLScene(QObject *parent = 0);
    virtual ~GLScene();

    //IO functions
    bool openImage(std::string fname);
    void saveImage(std::string fname);
    bool openCurves(std::string fname);
    void saveCurves(std::string fname);

    //Sketching functions
    void createBSpline();
    int registerPointAtScenePos(QPointF scenePos);

    cv::Mat& currentImage()
    {
        return m_curImage;
    }
    
protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    
public slots:
    void addCurveItem(int cid);
    void updateCurveItem(int cid);
    void addPointItem(int pid);

private:
    cv::Mat m_curImage;
    int m_curSplineIdx;

    QGraphicsPixmapItem *imageItem;
    QList<SplinePathItem *> curveItems;
    QList<ControlPointItem *> pointItems;
    SketchMode m_sketchmode;

public:
    BSplineGroup m_splineGroup;
};

class ControlPointItem : public QGraphicsEllipseItem
{
public:
    ControlPointItem(ControlPoint& _point):QGraphicsEllipseItem(), point(_point){}
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    ControlPoint& point;
};

class SplinePathItem: public QGraphicsPathItem
{
public:
    SplinePathItem(BSpline& _spline):QGraphicsPathItem(), spline(_spline){}
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    BSpline&  spline;
};

#endif // GLSCENE_H
