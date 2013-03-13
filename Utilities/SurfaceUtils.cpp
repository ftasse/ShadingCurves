#include "SurfaceUtils.h"
#include <QDebug>

QVector<ControlPoint> subDivide(QVector<ControlPoint> spline, int steps, bool closed)
{
    if(steps==0)
        return spline;
    QVector<ControlPoint> newVec;
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
        ControlPoint new1 = 0.5*spline.at(i-1)+0.5*spline.at(i);
        ControlPoint new2 = 0.125*spline.at(i-1)+0.75*spline.at(i)+0.125*spline.at(i+1);
        newVec.append(new1);
        newVec.append(new2);
    }

    newVec.append(0.5*spline.at(spline.count()-2)+0.5*spline.last());
    if(!closed)
        newVec.append(spline.last());

    newVec = subDivide(newVec,steps-1, closed);

    return newVec;
}

QVector<QPointF> limitPoints(QVector<ControlPoint> spline)
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

QPointF getNormal(QVector<ControlPoint> points, int index)
{
    QPointF tangent;
    if (index > 0 && index < points.size()-1) tangent = points[index+1] - points[index-1];
    else if (points.front() == points.back()) { // junction point
        QPointF vec1 = points[1] - points[0];
        QPointF vec2 = points[points.size()-2] - points[0];
        float d1 = sqrt(vec1.x()*vec1.x() + vec1.y()*vec1.y());
        float d2 = sqrt(vec2.x()*vec2.x() + vec2.y()*vec2.y());
        QPointF p1,p2;
        if(d1<d2) {
            p1 = points[1];
            p2 = (QPointF)points[0] + d1*(vec2/d2);
        } else {
            p1 = (QPointF)points[0] + d2*(vec1/d1);
            p2 = points[points.size()-2];
        }
        tangent = p1-p2;
    }
    else if (index == 0)
        tangent = points[index+1] - points[index];
    else
        tangent = points[index] - points[index-1];

    float norm = sqrt(tangent.x()*tangent.x() + tangent.y()*tangent.y());
    if (norm > 1e-5)
        tangent /= norm;
    QPointF normal(-tangent.y(), tangent.x());
    return normal;
}
