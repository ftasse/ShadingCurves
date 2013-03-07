#include "SurfaceUtils.h"
#include <QDebug>

QVector<QPointF> subDivide(QVector<QPointF> spline, bool closed, int steps)
{
    if(steps==0)
        return spline;
    QVector<QPointF> newVec;
    if(!closed||spline.count()<4)
        newVec.append(spline.first());

    // deal with splines with less than 4 control points
    if(spline.count()==2) {
        newVec.append(0.667*spline.first()+0.333*spline.last());
        newVec.append(0.333*spline.first()+0.667*spline.last());
        newVec.append(spline.last());
        newVec = subDivide(newVec,steps-1);
        return newVec;
    }

    if(spline.count()==3) {
        newVec.append(0.25*spline.first()+0.75*spline.at(1));
        newVec.append(0.25*spline.last()+0.75*spline.at(1));
        newVec.append(spline.last());
        newVec = subDivide(newVec,steps-1);
        return newVec;
    }

    for(int i = 1;i<spline.count()-1;i++)
    {
        QPointF new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        QPointF new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
        newVec.append(new1);
        newVec.append(new2);
    }

    newVec.append(0.5*spline.at(spline.count()-2)+0.5*spline.last());
    if(!closed)
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

QPointF getNormal(QVector<QPointF> points, int index)
{
    QPointF tangent;
    if (index > 0 && index < points.size()-1) tangent = points[index+1] - points[index-1];
    else if (points.front() == points.back()) tangent = points[1] - points[points.size() - 2]; // junction point
    else if (index == 0)    tangent = points[index+1] - points[index];
    else    tangent = points[index] - points[index-1];

    float norm = sqrt(tangent.x()*tangent.x() + tangent.y()*tangent.y());
    if (norm > 1e-5)
        tangent /= norm;
    QPointF normal(-tangent.y(), tangent.x());
    return normal;
}
