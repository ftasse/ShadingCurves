#include "ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/BSplineGroup.h"

ControlPoint::ControlPoint():
    Point3d(), ref(-1)
{
    useDefaultAttributes();
}

ControlPoint::ControlPoint(QPointF val):
    Point3d(val.x(), val.y()), ref(-1)
{
    useDefaultAttributes();
}

void ControlPoint::useDefaultAttributes()
{
    attributes[0].direction = INWARD_DIRECTION;
    attributes[1].extent = 50.0;
    attributes[1].height = 50.0;
    attributes[1].barycentricCoords = QPointF(0.0f, 0.0f);

    attributes[0].direction = OUTWARD_DIRECTION;
    attributes[1].extent = 50.0;
    attributes[1].height = -50.0;
    attributes[1].barycentricCoords = QPointF(0.0f, 0.0f);
}

BSpline& ControlPoint::splineAt(int index)
{
    int spline_ref = splineRefs[index];
    return m_splineGroup->spline(spline_ref);
}
