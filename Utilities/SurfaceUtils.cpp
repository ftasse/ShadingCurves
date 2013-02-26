#include "SurfaceUtils.h"

// NOTE: fix for degree<3
QVector<QPointF> subDivide(QVector<QPointF> spline, int steps)
{
    if(steps==0)
        return spline;

    QVector<QPointF> newVec;
    newVec.append(spline.first());
    for (int i=1;i<spline.count()-1;i++)
    {
        QPointF new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        QPointF new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
        newVec.append(new1);
        newVec.append(new2);
    }
    newVec.append(0.5*spline.at(spline.count()-2)+0.5*spline.last());
    newVec.append(spline.last());

    newVec = subDivide(newVec,steps-1);

    return newVec;
}

QVector<QPointF> limitPoints(QVector<QPointF> spline)
{
    QVector<QPointF> newVec;
    newVec.append(spline.first());
    for (int i=1;i<spline.count()-1;i++)
    {
        QPointF newP = 0.1667*spline.at(i-1)+0.667*spline.at(i)+0.1667*spline.at(i+1);
        newVec.append(newP);
    }
    newVec.append(spline.last());

    return newVec;
}

QVector<QPointF> normals(QVector<QPointF> spline)
{
    QVector<QPointF> newVec;
    newVec.append(spline.first());
    for (int i=1;i<spline.count()-1;i++)
    {
        QPointF newP = 0.1667*spline.at(i-1)+0.667*spline.at(i)+0.1667*spline.at(i+1);
        newVec.append(newP);
    }
    newVec.append(spline.last());

    return newVec;
}
