#ifndef POINT3D_H
#define POINT3D_H

#include <QPointF>
#include <QObject>

class Point3d : public QPointF
{
    Q_OBJECT
public:
    explicit Point3d(QObject *parent = 0);

    Point3d(float _x, float _y, float _z = 0.0);

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
    
signals:
    
public slots:
    
};

#endif // POINT3D_H
