#include "ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/BSplineGroup.h"

ControlPoint::ControlPoint():
    Point3d(), ref(-1)
{
}

ControlPoint::ControlPoint(QPointF val):
    Point3d(val.x(), val.y()), ref(-1)
{

}

BSpline& ControlPoint::splineAt(int index)
{
    int spline_ref = splineRefs[index];
    return m_splineGroup->spline(spline_ref);
}
