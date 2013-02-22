#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include <QPoint>
#include <sstream>
#include "../Curve/ControlPoint.h"

class Surface
{
public:
    Surface(QPoint degree = QPoint(3,3));

    ControlPoint& pointAt(QPoint pos);
    void updateKnotVectors();

    QVector< QVector<int> >& controlPoints()
    {
        return connected_cpts;
    }

    QPoint& degree()
    {
        return m_degree;
    }

    QVector<float>& u_knotVectors()
    {
        return m_knotVectors_u;
    }

    QVector<float>& v_knotVectors()
    {
        return m_knotVectors_v;
    }

    bool writeOFF(std::ostream &ofs);

    std::string surfaceToOFF()
    {
        std::stringstream ss;
        writeOFF(ss);
        return ss.str();
    }

public:
    BSplineGroup *m_splineGroup;
    int connected_spline_id;
    int idx;
    QVector< QVector<int> > connected_cpts;

private:
    QPoint m_degree;
    QPoint m_spec_degree;
    QVector<float> m_knotVectors_u;
    QVector<float> m_knotVectors_v;
};

#endif // SURFACE_H
