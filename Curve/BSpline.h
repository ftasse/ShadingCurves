#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include <QPainterPath>
#include "../Curve/ControlPoint.h"

class BSplineGroup;

class BSpline
{
public:
    BSpline( int degree = 3);
    void updateKnotVectors();

    //Utilites
    ControlPoint& pointAt(int index);

    QVector<float>& knotVectors()
    {
        return m_knotVectors;
    }

    unsigned int& degree()
    {
        return m_degree;
    }

    int count()
    {
        return connected_cpts.size();
    }

public:
    BSplineGroup *m_splineGroup;
    QVector<int> connected_cpts;
    int idx;

private:
    unsigned int m_spec_degree;
    unsigned int m_degree;
    QVector<float> m_knotVectors;
};

#endif // BSPLINE_H
