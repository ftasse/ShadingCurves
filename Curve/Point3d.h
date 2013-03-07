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

#endif // POINT3D_H
