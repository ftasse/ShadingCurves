#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QList>
#include <QPointF>
#include "Curve/BSpline.h"

class BSplineGroup;

class ControlPoint : public QPointF
{
public:
    ControlPoint();
    ControlPoint(QPointF val);

    BSpline& splineAt(int index);

    int count()
    {
        return connected_splines.size();
    }

public:
    BSplineGroup *m_splineGroup;
    QList<int> connected_splines;
    int idx;

private:
    bool is_valid;
};

#endif // CONTROLPOINT_H
