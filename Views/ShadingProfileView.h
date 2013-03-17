#ifndef SHADINGPROFILEITEM_H
#define SHADINGPROFILEITEM_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QDockWidget>
#include <QSlider>
#include <QGraphicsEllipseItem>
#include "Curve/ControlPoint.h"
#include "Curve/BSplineGroup.h"


class ShadingProfileView;
class ShadingProfileScene;

class ShapePointItem : public QGraphicsEllipseItem
{
public:
    NormalDirection direction;
    int index;
    int cpt_id;
    bool neg_x;
    bool neg_y;

    ShapePointItem() : QGraphicsEllipseItem()
    {
        direction = INWARD_DIRECTION;
        index = -1;
        cpt_id = -1;
        neg_x = neg_y = false;
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

class ShadingProfileScene : public QGraphicsScene
{
public:
    QGraphicsPathItem *redPathItem, *greenPathItem;
    QGraphicsPathItem *redPathItem_Info, *greenPathItem_Info;
    QVector<ShapePointItem *> redShapePoints, greenShapePoints;
    ShadingProfileView *shadingProfileView;

    ShadingProfileScene():QGraphicsScene()
    {
    }
protected:
    void keyPressEvent(QKeyEvent *event);
};

class ShadingProfileView : public QDockWidget
{
    Q_OBJECT

public:
    ShadingProfileView();

    QVector<int> cpts_ids;
    BSplineGroup *splineGroup;
    QWidget *centralWidget;

private:

    QGraphicsView* graphicsView;
    QSlider *inwardExtentWidget;
    QSlider *outwardExtentWidget;

    QSlider *inwardHeightWidget;
    QSlider *outwardHeightWidget;

    QPointF top, bottom, left, right;
    QRectF rect ;

    void propagateAttributes(ControlPoint& cpt);

signals:
    void controlPointAttributesChanged(int cptRef);

public slots:

    void updatePath();
    void refreshPath();

    void add_shape_point(QPointF point);
    void remove_shape_point(QVector<int> indices);
    void updateControlPointParameters();
};

#endif // SHADINGPROFILEITEM_H
