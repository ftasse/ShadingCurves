#ifndef SURFACEUTILS_H
#define SURFACEUTILS_H

#include <QVector>
#include <QPointF>

#include "Curve/BSpline.h"

QVector<ControlPoint> subDivide(QVector<ControlPoint> spline, int steps=2, bool closed = false);
QVector<QPointF> limitPoints(QVector<ControlPoint> spline);

QPointF getNormal(QVector<ControlPoint> points, int index);

#endif // SURFACEUTILS_H
