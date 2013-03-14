#ifndef SURFACEUTILS_H
#define SURFACEUTILS_H

#include <QVector>
#include <QPointF>

#include "Curve/BSpline.h"


QVector<QPointF> limitPoints(QVector<ControlPoint> spline);
QPointF getNormal(QVector<ControlPoint> points, int index);

template<class Vec>
QVector<Vec> subDivide(QVector<Vec> spline, int steps, bool closed)
{
    if(steps==0)
        return spline;
    QVector<Vec> newVec;
    if(!closed||spline.count()<4)
        newVec.append(spline.first());

    // deal with splines with less than 4 control points
    if(spline.count()==2) {
        newVec.append(0.667*spline.first()+0.333*spline.last());
        newVec.append(0.333*spline.first()+0.667*spline.last());
        newVec.append(spline.last());
        newVec = subDivide(newVec,steps-1, closed);
        return newVec;
    }

    if(spline.count()==3) {
        newVec.append(0.25*spline.first()+0.75*spline.at(1));
        newVec.append(0.25*spline.last()+0.75*spline.at(1));
        newVec.append(spline.last());
        newVec = subDivide(newVec,steps-1, closed);
        return newVec;
    }

    for(int i = 1;i<spline.count()-1;i++)
    {
        Vec new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        Vec new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
        newVec.append(new1);
        newVec.append(new2);
    }

    newVec.append(0.5*spline.at(spline.count()-2)+0.5*spline.last());
    if(!closed)
        newVec.append(spline.last());

    newVec = subDivide(newVec,steps-1, closed);

    return newVec;
}

#endif // SURFACEUTILS_H
