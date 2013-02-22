#ifndef SURFACEUTILS_H
#define SURFACEUTILS_H

#include <QVector>
#include <QPointF>

#include "Curve/BSpline.h"

QVector<QPointF> subDivide(QVector<QPointF> spline,int steps=2);

#endif // SURFACEUTILS_H
