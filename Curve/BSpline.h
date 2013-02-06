#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include <QPainterPath>
#include "Curve/ControlPoint.h"

class BSplineGroup;

class BSpline
{
public:
    BSpline();

    //Drawing
    void updatePath();

    //Utilites
    QPointF pointAt(int index);
    QPointF nextMiddlePoint(int index);

    QPainterPath& path()
    {
        return m_path;
    }

    int count()
    {
        return connected_cpts.size();
    }

public:
    BSplineGroup *m_splineGroup;
    QList<int> connected_cpts;
    int idx;

private:
    QPainterPath m_path;
    bool is_valid;
};

#endif // BSPLINE_H
