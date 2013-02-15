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

    //Normal at the (index)th control point
    QPointF inward_normal(int index);

    //Utilites
    ControlPoint& pointAt(int index);
    float closestParamToPointAt(int index);
    QPointF derivativeCurvePoint(float _t, unsigned int _der);
    float derivativeBasisFunction(int _i, int _n, double _t, int _der);
    float basisFunction(int _i, int _n, double _t);

    QPointF curvePoint(float _t)
    {
        return derivativeCurvePoint(_t, 0);
    }

    QVector<float>& knotVectors()
    {
        return m_knotVectors;
    }

    unsigned int& degree()
    {
        return m_degree;
    }

    bool is_closed()
    {
        return connected_cpts.front() == connected_cpts.back();
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
