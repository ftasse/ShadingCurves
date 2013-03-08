#ifndef SURFACEUTILS_H
#define SURFACEUTILS_H

#include <QVector>
#include <QPointF>

#include "Curve/BSpline.h"

QVector<QPointF> subDivide(QVector<QPointF> spline, int steps=2, bool closed = false);
QVector<QPointF> limitPoints(QVector<QPointF> spline);

QPointF getNormal(QVector<QPointF> points, int index);

#endif // SURFACEUTILS_H
