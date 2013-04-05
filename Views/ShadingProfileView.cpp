#include <QPainterPath>
#include <QVBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include "ShadingProfileView.h"

#define POINT_SIZE 10.0
#define PARAMETER_EXTENT 0
#define PARAMETER_HEIGHT 1
#define PARAMETER_SHAPE 2

void updateSliderLabelText(QScrollBar *slider, QLabel *label, std::string title, bool green)
{
    std::stringstream ss;

    if (green)
    {
        ss << "<font color='red'>";
    }
    else
    {
        ss << "<font color='green'>";
    }

    ss << title << " [" << slider->minimum() << " to " << slider->maximum() << "]" << "\n";
    ss << "Value: " << slider->value();

    ss << "</font>";

    label->setText(ss.str().c_str());
    label->setWordWrap(true);
}

QFrame* createHorizontalLine()
{
    QFrame* line = new QFrame();
    //line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(320, 150, 118, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
}

ShadingProfileView::ShadingProfileView()
{
    min_extent = max_extent = min_height = max_height = 0;

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

    inwardExtentWidget = new QScrollBar(Qt::Horizontal);    //inwardExtentWidget->setTracking(false);
    inwardExtentLabel = new QLabel ("Direction 1 Extent");
//    inwardExtentLabel->setText("<font color='green'>Direction 1 Extent</font>");
    outwardExtentWidget = new QScrollBar(Qt::Horizontal);   //outwardExtentWidget->setTracking(false);
    outwardExtentLabel = new QLabel ("Direction 2 Extent");
//    outwardExtentLabel->setText("<font color='red'>Direction 1 Extent</font>");
    inwardHeightWidget = new QScrollBar(Qt::Horizontal);    //inwardHeightWidget->setTracking(false);
    inwardHeightLabel = new QLabel ("Direction 1 Height");
//    inwardHeightLabel->setText("<font color='green'>Direction 1 Extent</font>");
    outwardHeightWidget = new QScrollBar(Qt::Horizontal);   //outwardHeightWidget->setTracking(false);
    outwardHeightLabel = new QLabel ("Direction 2 Height");
//    outwardHeightLabel->setText("<font color='red'>Direction 1 Extent</font>");

    childLayout2->addWidget(inwardExtentLabel); childLayout2->addWidget(inwardExtentWidget);

    childLayout2->addWidget(outwardExtentLabel); childLayout2->addWidget(outwardExtentWidget);
    childLayout2->addWidget(inwardHeightLabel); childLayout2->addWidget(inwardHeightWidget);
    childLayout2->addWidget(outwardHeightLabel); childLayout2->addWidget(outwardHeightWidget);
    childLayout2->addSpacing(30);

    connect(inwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateInwardExtent()));
    connect(outwardExtentWidget, SIGNAL(sliderReleased()), this, SLOT(updateOutwardExtent()));
    connect(inwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateInwardHeight()));
    connect(outwardHeightWidget, SIGNAL(sliderReleased()), this, SLOT(updateOutwardHeight()));

    connect(inwardExtentWidget, SIGNAL(valueChanged(int)), this, SLOT(updateInwardExtent()));
    connect(outwardExtentWidget, SIGNAL(valueChanged(int)), this, SLOT(updateOutwardExtent()));
    connect(inwardHeightWidget, SIGNAL(valueChanged(int)), this, SLOT(updateInwardHeight()));
    connect(outwardHeightWidget, SIGNAL(valueChanged(int)), this, SLOT(updateOutwardHeight()));

    splineGroup = NULL;
}

int ShadingProfileView::representativeCptRef()
{
    int mid = cpts_ids.size()/2;
    return cpts_ids[mid];
}

void ShadingProfileView::propagateAttributes(ControlPoint& cpt, NormalDirection direction, int parameter)
{

    for (int i=0; i<cpts_ids.size(); ++i)
    {
        if (cpts_ids[i] == cpt.ref) continue;
        ControlPoint& other = splineGroup->controlPoint(cpts_ids[i]);

        if (parameter == PARAMETER_HEIGHT) other.attribute(direction).height = cpt.attribute(direction).height;
        if (parameter == PARAMETER_EXTENT) other.attribute(direction).extent = cpt.attribute(direction).extent;
        if (parameter == PARAMETER_SHAPE) other.attribute(direction).shapePointAtr = cpt.attribute(direction).shapePointAtr;
    }

}

void ShadingProfileView::updateLabels()
{
    updateSliderLabelText(inwardExtentWidget, inwardExtentLabel, "Direction 1 Extent", false);
    updateSliderLabelText(outwardExtentWidget, outwardExtentLabel, "Direction 2 Extent", true);
    updateSliderLabelText(inwardHeightWidget, inwardHeightLabel, "Direction 1 Height", false);
    updateSliderLabelText(outwardHeightWidget, outwardHeightLabel, "Direction 2 Height", true);
}

void ShadingProfileView::updatePath()
{
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();
    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();
    my_scene->redPathItem = NULL;
    my_scene->greenPathItem = NULL;
    graphicsView->scene()->clear();

    if (splineGroup == NULL || cpts_ids.size() == 0)
        return;

    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());

    inwardExtentWidget->blockSignals( true );
    inwardExtentWidget->setMinimum(min_extent); inwardExtentWidget->setMaximum(max_extent);
    inwardExtentWidget->setValue((int)cpt.attributes[0].extent);
    inwardExtentWidget->blockSignals( false );

    outwardExtentWidget->blockSignals( true );
    outwardExtentWidget->setMinimum(min_extent); outwardExtentWidget->setMaximum(max_extent);
    outwardExtentWidget->setValue((int)cpt.attributes[1].extent);
    outwardExtentWidget->blockSignals( false );

    inwardHeightWidget->blockSignals( true );
    inwardHeightWidget->setMinimum(min_height); inwardHeightWidget->setMaximum(max_height);
    inwardHeightWidget->setValue((int)cpt.attributes[0].height);
    inwardHeightWidget->blockSignals( false );

    outwardHeightWidget->blockSignals( true );
    outwardHeightWidget->setMinimum(min_height); outwardHeightWidget->setMaximum(max_height);
    outwardHeightWidget->setValue((int)cpt.attributes[1].height);
    outwardHeightWidget->blockSignals( false );

    updateLabels();

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
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
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
        if (cpt.attributes[0].height >= 0)   P1 =top;
        else P1 = bottom;
        QPainterPath path;
        if (my_scene->greenShapePoints.size() == 1)
        {
            path.moveTo(P3);
            path.quadTo(gPoints[0], P1);

        } else if (my_scene->greenShapePoints.size()>1)
        {
            path.moveTo(P3);
            path.cubicTo(gPoints[0], gPoints[1], P1);
        }
        QPainterPath path2;
        path2.moveTo(P3);
        for (int k=0; k<my_scene->greenShapePoints.size(); ++k)
            path2.lineTo(gPoints[k]);
        path2.lineTo(P1);

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
        if (cpt.attributes[1].height >= 0)   P1 =top;
        else P1 = bottom;
        QPainterPath path;
        if (my_scene->redShapePoints.size() == 1)
        {
            path.moveTo(P3);
            path.quadTo(rPoints[0], P1);
        } else if (my_scene->redShapePoints.size()>1)
        {
            path.moveTo(P3);
            path.cubicTo(rPoints[0], rPoints[1], P1);
        }
        QPainterPath path2;
        path2.moveTo(P3);
        for (int k=0; k<my_scene->redShapePoints.size(); ++k)
            path2.lineTo(rPoints[k]);
        path2.lineTo(P1);

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
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();

    for (int k=0; k<2; ++k)
    {
        cpt.attributes[k].shapePointAtr.push_back(point);
    }

    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();
    refreshPath();
    updateShape(true, true);
}

void ShadingProfileView::remove_shape_point(QVector<int> indices)
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    ShadingProfileScene *my_scene = (ShadingProfileScene*) graphicsView->scene();

    for (int k=0; k<2; ++k)
    {
        for (int l=0; l<indices.size(); ++l)
            cpt.attributes[k].shapePointAtr.erase(cpt.attributes[k].shapePointAtr.begin()+indices[l] - l);
    }

    for (int i=0; i<my_scene->greenShapePoints.size(); ++i) my_scene->removeItem(my_scene->greenShapePoints[i]);
    for (int i=0; i<my_scene->redShapePoints.size(); ++i) my_scene->removeItem(my_scene->redShapePoints[i]);
    my_scene->greenShapePoints.clear();
    my_scene->redShapePoints.clear();

    refreshPath();
    updateShape(true, true);
}

void ShadingProfileView::updateInwardHeight()
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    cpt.attributes[0].height = inwardHeightWidget->value();
    if (inwardHeightWidget->isSliderDown())
    {
        updateLabels();
        return;
    }
    propagateAttributes(cpt, INWARD_DIRECTION, PARAMETER_HEIGHT);
    updatePath();
    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileView::updateOutwardHeight()
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    cpt.attributes[1].height = outwardHeightWidget->value();
    if (outwardHeightWidget->isSliderDown())
    {
        updateLabels();
        return;
    }
    propagateAttributes(cpt, OUTWARD_DIRECTION, PARAMETER_HEIGHT);
    updatePath();
    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileView::updateInwardExtent()
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    cpt.attributes[0].extent = inwardExtentWidget->value();
    if (inwardExtentWidget->isSliderDown())
    {
        updateLabels();
        return;
    }
    propagateAttributes(cpt, INWARD_DIRECTION, PARAMETER_EXTENT);
    updatePath();
    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileView::updateOutwardExtent()
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    cpt.attributes[1].extent = outwardExtentWidget->value();
    if (outwardExtentWidget->isSliderDown())
    {
        updateLabels();
        return;
    }
    propagateAttributes(cpt, OUTWARD_DIRECTION, PARAMETER_EXTENT);
    updatePath();
    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileView::updateShape(bool update_inward, bool update_outward)
{
    ControlPoint& cpt = splineGroup->controlPoint(representativeCptRef());
    if (update_inward) propagateAttributes(cpt, INWARD_DIRECTION, PARAMETER_SHAPE);
    if (update_outward) propagateAttributes(cpt, OUTWARD_DIRECTION, PARAMETER_SHAPE);
    updatePath();
    emit controlPointAttributesChanged(cpt.ref);
}

void ShadingProfileScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    //if (event->isAccepted())    return;

    if (edited_inward || edited_outward)
    {
        shadingProfileView->updateShape(edited_inward, edited_outward);
        edited_inward = edited_outward = false;
    }
}

void ShadingProfileScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);

    for (int i=0; i<greenShapePoints.size(); ++i)
    {
        ShapePointItem *shapeItem = greenShapePoints[i];
        if (!shapeItem->isSelected())   continue;

        QPointF diff = event->scenePos() - event->lastScenePos();
        if (shapeItem->neg_x) diff.setX(-diff.x());
        if (shapeItem->neg_y) diff.setY(-diff.y());
        diff /= 100;
        ControlPoint& cpt = shadingProfileView->splineGroup->controlPoint(shapeItem->cpt_id);
        cpt.attribute(shapeItem->direction).shapePointAtr[shapeItem->index] += diff;
        edited_inward = true;
    }

    for (int i=0; i<redShapePoints.size(); ++i)
    {
        ShapePointItem *shapeItem = redShapePoints[i];
        if (!shapeItem->isSelected())   continue;

        QPointF diff = event->scenePos() - event->lastScenePos();
        if (shapeItem->neg_x) diff.setX(-diff.x());
        if (shapeItem->neg_y) diff.setY(-diff.y());
        diff /= 100;
        ControlPoint& cpt = shadingProfileView->splineGroup->controlPoint(shapeItem->cpt_id);
        cpt.attribute(shapeItem->direction).shapePointAtr[shapeItem->index] += diff;
        edited_outward = true;
    }

    shadingProfileView->refreshPath();
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

/*QVariant ShapePointItem::itemChange(GraphicsItemChange change, const QVariant &value)
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
 }*/
