#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QList>
#include <QVector>

#include "Point3d.h"

class BSplineGroup;
class BSpline;

typedef struct Attribute
{
    int type; //0 for inward point along the normal,    1 for outward point along the normal
    float extend;
    float height;
    QPointF barycentricCoords;
} Attribute;

class ControlPoint : public Point3d
{
public:
    ControlPoint();
    ControlPoint(QPointF val);

    BSpline& splineAt(int index);

    int num_splines()
    {
        return splineRefs.size();
    }

public:
    BSplineGroup *m_splineGroup;
    QVector<int> splineRefs;

    int ref;
    QVector<Attribute> attributes;
};

#endif // CONTROLPOINT_H
