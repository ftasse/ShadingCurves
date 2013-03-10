#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <QList>
#include <QVector>

#include "Point3d.h"

class BSplineGroup;
class BSpline;

typedef enum NormalDirection
{
  INWARD_DIRECTION = 0,
  OUTWARD_DIRECTION
} NormalDirection;

typedef struct Attribute
{
    NormalDirection direction; //0 for inward point along the normal,    1 for outward point along the normal
    float extent;
    float height;
    QPointF shapePointAtr;
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

    Attribute& attribute(NormalDirection dir)
    {
        if (dir == INWARD_DIRECTION)
            return attributes[0];
        else
            return attributes[2];
    }

    void useDefaultAttributes();

public:
    BSplineGroup *m_splineGroup;
    QVector<int> splineRefs;

    int ref;
    Attribute attributes[2];
};

#endif // CONTROLPOINT_H
