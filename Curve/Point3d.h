#ifndef POINT3D_H
#define POINT3D_H

#include <QPointF>

class Point3d : public QPointF
{
public:
    Point3d(float _x = 0.0, float _y = 0.0, float _z = 0.0);

    float z()
    {
        return m_z;
    }

    void setZ(float _z)
    {
        m_z = _z;
    }

private:
    float m_z;
    
};

inline Point3d operator+(Point3d p1, Point3d p2)
{
  return Point3d(p1.x()+p2.x(), p1.y()+p2.y(), p1.z()+p2.z());
}

inline Point3d operator*(qreal a, Point3d p2)
{
  return Point3d(a*p2.x(), a*p2.y(), a*p2.z());
}

#endif // POINT3D_H
