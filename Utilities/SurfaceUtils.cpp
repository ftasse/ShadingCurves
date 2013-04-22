#include "SurfaceUtils.h"
#include <QDebug>


QVector<QPointF> limitPoints(QVector<ControlPoint> spline)
{
    QVector<QPointF> newVec;
    newVec.append(spline.first());
    for (int i=1;i<spline.count()-1;i++)
    {
        QPointF newP;
        if(spline[i].isSharp)
            newP = spline[i];
        else
            newP = 0.1667*spline.at(i-1)+0.667*spline.at(i)+0.1667*spline.at(i+1);
        newVec.append(newP);
    }
    newVec.append(spline.last());

    return newVec;
}

QPointF getNormal(QVector<ControlPoint> points, int index)
{
    QPointF tangent;
    if (index > 0 && index < points.size()-1)
    {
        float norm;

        QPointF v1 =  points[index+1] - points[index];
        norm = sqrt(v1.x()*v1.x() + v1.y()*v1.y());
        if (norm > 1e-5)    v1 /= norm;

        QPointF v2 =  points[index-1] - points[index];
        norm = sqrt(v2.x()*v2.x() + v2.y()*v2.y());
        if (norm > 1e-5)    v2 /= norm;

        tangent = v1 - v2;
    }
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
