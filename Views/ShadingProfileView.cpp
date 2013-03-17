#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include "ShadingProfileView.h"

#define POINT_SIZE 10.0

ShadingProfileView::ShadingProfileView()
{
    centralWidget = new QWidget();
    setWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    centralWidget->setLayout(layout);
    graphicsView = new QGraphicsView ();
    graphicsView->setScene(new ShadingProfileScene ());
    ShadingProfileScene *my_scene = (ShadingProfileScene *)graphicsView->scene();
    my_scene->shadingProfileView = this;

    QVBoxLayout *childLayout1 = new QVBoxLayout;    layout->addLayout(childLayout1);
    QVBoxLayout *childLayout2 = new QVBoxLayout;    layout->addLayout(childLayout2);

    childLayout1->addWidget(graphicsView);
    childLayout1->addSpacing(30);

    inwardExtentWidget = new QSlider(Qt::Horizontal);   inwardExtentWidget->setMinimum(0);  inwardExtentWidget->setMaximum(200);
    outwardExtentWidget = new QSlider(Qt::Horizontal);  outwardExtentWidget->setMinimum(0);  outwardExtentWidget->setMaximum(200);
    inwardHeightWidget = new QSlider(Qt::Horizontal);   inwardHeightWidget->setMinimum(-100);  inwardExtentWidget->setMaximum(100);
    outwardHeightWidget = new QSlider(Qt::Horizontal);  outwardHeightWidget->setMinimum(-100);  outwardExtentWidget->setMaximum(100);
    childLayout2->addWidget(new QLabel("In  Extent (0:200)")); childLayout2->addWidget(inwardExtentWidget);
    childLayout2->addWidget(new QLabel("Out Extent (0:200)")); childLayout2->addWidget(outwardExtentWidget);
    childLayout2->addWidget(new QLabel("In  Height (-100:100)")); childLayout2->addWidget(inwardHeightWidget);
    childLayout2->addWidget(new QLabel("Out Height (-100:100)")); childLayout2->addWidget(outwardHeightWidget);
    childLayout2->addSpacing(30);

    connect(inwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(outwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(inwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));
    connect(outwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateControlPointParameters()));

    splineGroup = NULL;
}

void ShadingProfileView::propagateAttributes(ControlPoint& cpt)
{

    for (int i=0; i<cpts_ids.size(); ++i)
    {
        ControlPoint& other = splineGroup->controlPoint(cpts_ids[i]);

        for (int k=0; k<2; ++k)
        {
            other.attributes[k].height = cpt.attributes[k].height;
            other.attributes[k].extent = cpt.attributes[k].extent;
            other.attributes[k].shapePointAtr = cpt.attributes[k].shapePointAtr;
        }
    }

    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileView::updatePath()
{
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();
    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();
    my_scene->redPathItem = NULL;
    my_scene->greenPathItem = NULL;
    graphicsView->scene()->clear();

    if (splineGroup == NULL || cpts_ids.size() < 0)
        return;

    ControlPoint& cpt = splineGroup->controlPoint(cpts_ids.first());
    //propagateAttributes(cpt);

    inwardExtentWidget->setValue((int)cpt.attributes[0].extent);
    outwardExtentWidget->setValue((int)cpt.attributes[1].extent);
    inwardHeightWidget->setValue((int)cpt.attributes[0].height);
    outwardHeightWidget->setValue((int)cpt.attributes[1].height);

    rect = QRectF(0,0, 200, 200); // graphicsView->scene()->sceneRect();
    top = QPointF(rect.x() + rect.width()/2, rect.y());
    bottom = QPointF(rect.x() + rect.width()/2, rect.y()+rect.height());
    left = QPointF(rect.x(), rect.y() + rect.height()/2);
    right = QPointF(rect.x() + rect.width(), rect.y() + rect.height()/2);
    float ps = POINT_SIZE;

    graphicsView->scene()->addEllipse(top.x()-ps/2, top.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(bottom.x()-ps/2, bottom.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(left.x()-ps/2, left.y()-ps/2, ps, ps);
    graphicsView->scene()->addEllipse(right.x()-ps/2, right.y()-ps/2, ps, ps);

    QPainterPath path;
    path.moveTo(top.x(), top.y()); path.lineTo(bottom.x(), bottom.y());
    path.closeSubpath();
    path.moveTo(left.x(), left.y()); path.lineTo(right.x(), right.y());

    graphicsView->scene()->addPath(path, QPen(QColor(79, 106, 25), 1, Qt::SolidLine,Qt::FlatCap, Qt::MiterJoin),
                     QColor(122, 163, 39));

    refreshPath();
}

void ShadingProfileView::refreshPath()
{
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();
    ControlPoint& cpt = splineGroup->controlPoint(cpts_ids.first());
    float ps = POINT_SIZE;

    QVector<QPointF> gPoints, rPoints;

    for (int i=0; i<cpt.attributes[0].shapePointAtr.size(); ++i)
    {
        QPointF shapePoint = cpt.attributes[0].shapePointAtr[i];
        shapePoint *= rect.width()/2;
        shapePoint.setX(shapePoint.x() + rect.width()/2);
        if (cpt.attributes[0].height < 0)   shapePoint.setY(shapePoint.y() + rect.width()/2);
        else shapePoint.setY(-shapePoint.y() + rect.width()/2);
        gPoints.push_back(shapePoint);

        if (i+1 > (int)my_scene->greenShapePoints.size())
        {
            ShapePointItem *item = new ShapePointItem ();
            item->setRect(shapePoint.x()-ps/2, shapePoint.y()-ps/2, ps, ps);
            item->cpt_id = cpt.ref;
            item->direction = cpt.attributes[0].direction;
            item->index = i;
            item->neg_x = false;
            if (cpt.attributes[0].height < 0) item->neg_y = false;
            else item->neg_y = true;

            int color = (((float)i+1)/cpt.attributes[0].shapePointAtr.size())*255;
            item->setBrush(QBrush(QColor(0,color, 0)));
            my_scene->addItem(item);
            my_scene->greenShapePoints.push_back(item);
            my_scene->greenShapePoints.last()->setFlags(QGraphicsItem::ItemIsMovable |
                                                        QGraphicsItem::ItemIsSelectable |
                                                        QGraphicsItem::ItemSendsScenePositionChanges |
                                                        QGraphicsItem::ItemSendsGeometryChanges);
        } //else my_scene->greenShapePoints[i]->setRect(shapePoint.x()-ps/2, shapePoint.y()-ps/2, ps, ps);
    }
    {
        QPointF P1, P3 = right;
        if (cpt.attributes[0].height > 0)   P1 =top;
        else P1 = bottom;
        QPainterPath path;
        if (my_scene->greenShapePoints.size() == 1)
        {
            path.moveTo(P1);
            path.quadTo(gPoints[0], P3);

        } else if (my_scene->greenShapePoints.size()>1)
        {
            path.moveTo(P1);
            path.cubicTo(gPoints[0], gPoints[1], P3);
        }
        QPainterPath path2;
        path2.moveTo(P1);
        for (int k=0; k<my_scene->greenShapePoints.size(); ++k)
            path2.lineTo(gPoints[k]);
        path2.lineTo(P3);

        if (my_scene->greenPathItem == NULL)
        {
            my_scene->greenPathItem = my_scene->addPath(path, QPen(Qt::green, 2, Qt::SolidLine));
            my_scene->greenPathItem_Info = my_scene->addPath(path2, QPen(Qt::lightGray, 1, Qt::SolidLine));
        }
        else
        {
            my_scene->greenPathItem->setPath(path);
            my_scene->greenPathItem_Info->setPath(path2);
        }
    }

    for (int i=0; i<cpt.attributes[1].shapePointAtr.size(); ++i)
    {
        QPointF shapePoint = cpt.attributes[1].shapePointAtr[i];
        shapePoint *= rect.width()/2;
        shapePoint.setX(-shapePoint.x() + rect.width()/2);
        if (cpt.attributes[1].height < 0)   shapePoint.setY(shapePoint.y() + rect.width()/2);
        else shapePoint.setY(-shapePoint.y() + rect.width()/2);
        rPoints.push_back(shapePoint);

        if  (i+1>(int)my_scene->redShapePoints.size())
        {
            ShapePointItem *item = new ShapePointItem ();
            item->setRect(shapePoint.x()-ps/2, shapePoint.y()-ps/2, ps, ps);
            item->cpt_id = cpt.ref;
            item->direction = cpt.attributes[1].direction;
            item->index = i;
            item->neg_x = true;
            if (cpt.attributes[1].height < 0) item->neg_y = false;
            else item->neg_y = true;

            int color = (((float)i+1)/cpt.attributes[1].shapePointAtr.size())*255;
            item->setBrush(QBrush(QColor(color,0,0)));
            graphicsView->scene()->addItem(item);
            my_scene->redShapePoints.push_back(item);
            my_scene->redShapePoints.last()->setFlags(QGraphicsItem::ItemIsMovable |
                                                      QGraphicsItem::ItemIsSelectable |
                                                      QGraphicsItem::ItemSendsScenePositionChanges |
                                                      QGraphicsItem::ItemSendsGeometryChanges);
        }   //else my_scene->redShapePoints[i]->setRect(shapePoint.x()-ps/2, shapePoint.y()-ps/2, ps, ps);
    }
    {
        QPointF P1, P3 = left;
        if (cpt.attributes[1].height > 0)   P1 =top;
        else P1 = bottom;
        QPainterPath path;
        if (my_scene->redShapePoints.size() == 1)
        {
            path.moveTo(P1);
            path.quadTo(rPoints[0], P3);
        } else if (my_scene->redShapePoints.size()>1)
        {
            path.moveTo(P1);
            path.cubicTo(rPoints[0], rPoints[1], P3);
        }
        QPainterPath path2;
        path2.moveTo(P1);
        for (int k=0; k<my_scene->redShapePoints.size(); ++k)
            path2.lineTo(rPoints[k]);
        path2.lineTo(P3);

        if (my_scene->redPathItem == NULL)
        {
            my_scene->redPathItem = graphicsView->scene()->addPath(path, QPen(Qt::red, 2, Qt::SolidLine));
            my_scene->redPathItem_Info = graphicsView->scene()->addPath(path2, QPen(Qt::lightGray, 1, Qt::SolidLine));
        }
        else
        {
            my_scene->redPathItem->setPath(path);
            my_scene->redPathItem_Info->setPath(path2);
        }
    }
    update();
}

void ShadingProfileView::add_shape_point(QPointF point)
{
    ControlPoint& cpt = splineGroup->controlPoint(cpts_ids.first());
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();

    for (int k=0; k<2; ++k)
    {
        cpt.attributes[k].shapePointAtr.push_back(point);
    }

    propagateAttributes(cpt);
    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();
    refreshPath();
}

void ShadingProfileView::remove_shape_point(QVector<int> indices)
{
    ControlPoint& cpt = splineGroup->controlPoint(cpts_ids.first());
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();

    for (int k=0; k<2; ++k)
    {
        for (int l=0; l<indices.size(); ++l)
            cpt.attributes[k].shapePointAtr.erase(cpt.attributes[k].shapePointAtr.begin()+indices[l] - l);
    }

    propagateAttributes(cpt);

    for (int i=0; i<my_scene->greenShapePoints.size(); ++i) my_scene->removeItem(my_scene->greenShapePoints[i]);
    for (int i=0; i<my_scene->redShapePoints.size(); ++i) my_scene->removeItem(my_scene->redShapePoints[i]);
    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();

    refreshPath();
}

void ShadingProfileView::updateControlPointParameters()
{
    ControlPoint& cpt = splineGroup->controlPoint(cpts_ids.first());

   cpt.attributes[0].extent = inwardExtentWidget->value();
   cpt.attributes[1].extent = outwardExtentWidget->value();
   cpt.attributes[0].height = inwardHeightWidget->value();
   cpt.attributes[1].height = outwardHeightWidget->value();

   propagateAttributes(cpt);
   refreshPath();
}

void ShadingProfileScene::keyPressEvent(QKeyEvent *event)
{
    QGraphicsScene::keyPressEvent(event);
    if (event->isAccepted())   return;

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        QVector<int> indices;
        for (int k=0; k<redShapePoints.size(); ++k)
        {
            if (redShapePoints[k]->isSelected()) indices.push_back(redShapePoints[k]->index);
        }
        for (int k=0; k<greenShapePoints.size(); ++k)
        {
            if (greenShapePoints[k]->isSelected()) indices.push_back(greenShapePoints[k]->index);
        }

        std::sort(indices.begin(), indices.end());
        indices.erase( std::unique( indices.begin(), indices.end() ), indices.end() );

        shadingProfileView->remove_shape_point(indices);
        event->accept();
        return;
    } else if (event->key() == Qt::Key_D)
    {
        shadingProfileView->add_shape_point(QPointF(0.5, 0.5));
        event->accept();
    } else if (event->key() == Qt::Key_U)
    {
        shadingProfileView->updatePath();
        event->accept();
    }
}

QVariant ShapePointItem::itemChange(GraphicsItemChange change, const QVariant &value)
 {
     if (change == ItemPositionChange && scene()) {
         // value is the new position.
         QPointF newPos = value.toPointF();
         QPointF diff = newPos - pos();

         if (neg_x) diff.setX(-diff.x());
         if (neg_y) diff.setY(-diff.y());
         diff /= 100;

         ShadingProfileView *view = ((ShadingProfileScene *)scene())->shadingProfileView;
         view->splineGroup->controlPoint(cpt_id).attribute(direction).shapePointAtr[index] += diff;
     } else if (change == QGraphicsItem::ItemPositionHasChanged && scene())
     {
         ShadingProfileView *view = ((ShadingProfileScene *)scene())->shadingProfileView;
         view->refreshPath();
     }
     return QGraphicsItem::itemChange(change, value);
 }
