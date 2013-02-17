#include "ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/BSplineGroup.h"

ControlPoint::ControlPoint():
    QPointF(), idx(-1),mz(0), isVisible(true)
{
}

ControlPoint::ControlPoint(QPointF val):
    QPointF(val), idx(-1),mz(0), isVisible(true)
{

}


BSpline& ControlPoint::splineAt(int index)
{
    int spline_idx = connected_splines[index];
    return m_splineGroup->spline(spline_idx);
}
