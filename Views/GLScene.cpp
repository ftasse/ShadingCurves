#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <GL/glu.h>
#include <algorithm>

#include "GLScene.h"

GLScene::GLScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_curSplineIdx = -1;
    m_sketchmode = IDLE_MODE;
    imageItem = NULL;

    pointSize = 8.0;
}

GLScene:: ~GLScene()
{
    if (imageItem) delete imageItem;
}

void GLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_sketchmode == ADD_CURVE_MODE)
    {
        int cptRef = registerPointAtScenePos(event->scenePos());
        if (cptRef < 0)
            return;

        if (m_curSplineIdx < 0)
        {
            createBSpline();
        }

        if (m_splineGroup.addControlPoint(m_curSplineIdx, cptRef))
        {
            updateCurveItem(m_curSplineIdx);
        }
    } else
    {
        QGraphicsScene::mousePressEvent(event);
    }
}

void GLScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        for (int i=0; i<pointItems.size();)
        {

            /*TODO Flora
              Deletion is buggy.
              Need to reassign indexes properly,
              */
            if (pointItems[i]->isSelected())
            {
                QList<int> itemsToUpdate;
                for (int k=0; k<pointItems[i]->point.count(); ++k)
                {
                    itemsToUpdate.append(pointItems[i]->point.splineAt(k).idx);
                }
                removeItem(pointItems[i]);
                m_splineGroup.removeControlPoint(pointItems[i]->point.idx);

                pointItems.removeAt(i);
                for (int k=0; k<itemsToUpdate.size(); ++k)
                {
                    updateCurveItem(itemsToUpdate.at(k));
                }

            } else
            {
                 ++i;
            }
        }
        update();

    } else if (m_sketchmode == ADD_CURVE_MODE)
    {
        switch(event->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            {
                m_sketchmode = IDLE_MODE;
                break;
            }
            default:
                break;
        }

    } else
    {
        QGraphicsScene::keyPressEvent(event);
    }
}

void  GLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
}

void GLScene::createBSpline()
{
    m_curSplineIdx = m_splineGroup.addBSpline();
    addCurveItem(m_curSplineIdx);
    m_sketchmode = ADD_CURVE_MODE;
}

int GLScene::registerPointAtScenePos(QPointF scenePos)
{
    QGraphicsItem *item = itemAt(scenePos.x(), scenePos.y());
    if (item)
    {
        for (int i=0; i<pointItems.size(); ++i)
        {
            if (item == pointItems[i])
            {
                return pointItems[i]->point.idx;
            }
        }
    }
    int pointIdx = m_splineGroup.addControlPoint(scenePos);
    addPointItem(pointIdx);
    return pointIdx;
}

bool GLScene::openImage(std::string fname)
{
    cv::Mat image = loadImage(fname);

    //Test if image was loaded
    if (image.cols > 0)
    {
        m_curImage = image;

        QImage::Format format = QImage::Format_RGB888;
        QImage img_qt = QImage((const unsigned char*)m_curImage.data,
                                m_curImage.cols, m_curImage.rows,
                                m_curImage.step, format);
        img_qt = img_qt.rgbSwapped();
        QPixmap pixmap = QPixmap::fromImage(img_qt);
        if (!imageItem)  imageItem = addPixmap(pixmap);
        else imageItem->setPixmap(pixmap);

        setSceneRect(0, 0, m_curImage.cols, m_curImage.rows);

        return true;
    }
    else
        return false;
}

void GLScene::saveImage(std::string fname)
{
    cv::imwrite(fname, m_curImage);
}

bool GLScene::openCurves(std::string fname)
{
    if (m_splineGroup.load(fname))
    {
        for (int i=0; i<m_splineGroup.num_controlPoints(); ++i)
        {
            addPointItem(i);
        }
        for (int i=0; i<m_splineGroup.num_splines(); ++i)
        {
            addCurveItem(i);
        }
        return true;

    } else
    {
        return false;
    }
}

void GLScene::saveCurves(std::string fname)
{
    m_splineGroup.save(fname);
}

void GLScene::addCurveItem(int cid)
{
    BSpline &bspline = m_splineGroup.spline(cid);
    SplinePathItem *curveItem = new SplinePathItem(bspline);
    curveItem->setPath(bspline.path());
    /*curveItem->setFlags(QGraphicsItem::ItemIsMovable |
                        QGraphicsItem::ItemIsSelectable |
                        QGraphicsItem::ItemSendsScenePositionChanges);*/
    addItem(curveItem);
    curveItems.push_back(curveItem);
}

void GLScene::updateCurveItem(int cid)
{
    for (int i=0; i<curveItems.size(); ++i)
    {
        SplinePathItem *curveItem = curveItems.at(i);
        if (curveItem->spline.idx == cid)
        {
            curveItem->setPath(curveItem->spline.path());
            break;
        }
    }
}

void GLScene::addPointItem(int pid)
{
    ControlPoint& cpt = m_splineGroup.controlPoint(pid);
    ControlPointItem *pointItem = new ControlPointItem (cpt);
    pointItem->setRect(-pointSize/2.0,-pointSize/2, pointSize, pointSize);
    pointItem->setPos(cpt.x(), cpt.y());
    pointItem->setBrush(QBrush(Qt::black));
    pointItem->setFlags(QGraphicsItem::ItemIsMovable |
                        QGraphicsItem::ItemIsSelectable |
                        QGraphicsItem::ItemSendsScenePositionChanges);
    addItem(pointItem);
    pointItems.push_back(pointItem);
}

void GLScene::updatePointItems()
{
    for (int i=0; i< pointItems.size(); ++i)
    {
        pointItems[i]->setRect(-pointSize/2.0,-pointSize/2, pointSize, pointSize);
    }
}

QVariant SplinePathItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange && scene())
    {
        //QPointF newPos = value.toPointF();
    }

    return QGraphicsPathItem::itemChange(change, value);
}

QVariant ControlPointItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged && scene())
    {
        QPointF newPos = scenePos();
        {
            GLScene *my_scene = (GLScene *)scene();
            /*qDebug("%f  %f <-- %f %f / %f %f", newPos.x(), newPos.y(),
                   cpt.x(), cpt.y(),
                   x(), y());*/
            point.setX(newPos.x());
            point.setY(newPos.y());

            for (int i=0; i<point.count(); ++i)
            {
                point.splineAt(i).updatePath();
                my_scene->updateCurveItem(point.splineAt(i).idx);
            }
            my_scene->update();
        }
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}
