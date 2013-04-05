#ifndef SHADINGPROFILEITEM_H
#define SHADINGPROFILEITEM_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QDockWidget>
#include <QScrollBar>
#include <QLabel>
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

    //QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

class ShadingProfileScene : public QGraphicsScene
{
public:
    QGraphicsPathItem *redPathItem, *greenPathItem;
    QGraphicsPathItem *redPathItem_Info, *greenPathItem_Info;
    QVector<ShapePointItem *> redShapePoints, greenShapePoints;
    ShadingProfileView *shadingProfileView;
    bool edited_inward, edited_outward;

    ShadingProfileScene():QGraphicsScene()
    {
        edited_inward = edited_outward = false;
    }
protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

class ShadingProfileView : public QDockWidget
{
    Q_OBJECT

public:
    ShadingProfileView();

    QVector<int> cpts_ids;
    BSplineGroup *splineGroup;
    QWidget *centralWidget;

    int min_extent, max_extent;
    int min_height, max_height;

private:

    QGraphicsView* graphicsView;
    QScrollBar *inwardExtentWidget;
    QScrollBar *outwardExtentWidget;

    QScrollBar *inwardHeightWidget;
    QScrollBar *outwardHeightWidget;

    QLabel *inwardExtentLabel;
    QLabel *outwardExtentLabel;
    QLabel *inwardHeightLabel;
    QLabel *outwardHeightLabel;

    QPointF top, bottom, left, right;
    QRectF rect ;

    int representativeCptRef();
    void propagateAttributes(ControlPoint& cpt, NormalDirection direction, int parameter);

protected:

signals:
    void controlPointAttributesChanged(int cptRef);

public slots:

    void updatePath();
    void updateLabels();
    void refreshPath();

    void add_shape_point(QPointF point);
    void remove_shape_point(QVector<int> indices);
    void updateInwardHeight();
    void updateOutwardHeight();
    void updateInwardExtent();
    void updateOutwardExtent();
    void updateShape(bool update_inward, bool update_outward);
};

#endif // SHADINGPROFILEITEM_H
