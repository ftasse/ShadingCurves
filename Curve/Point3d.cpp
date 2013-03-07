#include "Point3d.h"

Point3d::Point3d(QObject *parent) :
    QPointF(parent), _z(0.0f)
{
}

Point3d::Point3d(float _x, float _y, float _z) :
    QPointF(_x, _y)
{
    setZ(_z);
}
