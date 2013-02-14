#include "BSpline.h"
#include "BSplineGroup.h"
#include <cmath>

BSpline::BSpline(int degree):
    idx(-1), m_spec_degree(degree), m_degree(degree)
{
}

void BSpline::updateKnotVectors()
{
    //Uniform knots
    m_knotVectors.clear();
    if (count() == 0)
        return;

    if (count() > (int)m_spec_degree)
        m_degree = m_spec_degree;
    else
        m_degree = count()-1;

    float last=0.0;
    for (unsigned int i = 0; i < m_degree; i++) m_knotVectors.push_back(0.0);
    for (int i = 0; i < (int)count() - (int)m_degree + 1; i++) {
        m_knotVectors.push_back(i);
        last=i;
    }
    for (unsigned int i = 0; i < m_degree; i++)
        m_knotVectors.push_back(last);
}

ControlPoint& BSpline::pointAt(int index)
{
    int cpt_idx = connected_cpts[index];
    return m_splineGroup->controlPoint(cpt_idx);
}

QPointF BSpline::derivativeCurvePoint(float _t, unsigned int _der)
{
    int n = count(); // number of control points
    int p = degree();           // spline degree

    QPointF point(0.0, 0.0);
    for (int i = 0; i < n; i++)
    {
        double bf = derivativeBasisFunction(i, p, _t, _der);
        point += pointAt(i) * bf;
    }
    return point;
}

float BSpline::basisFunction(int _i, int _n, double _t)
{
    int m = m_knotVectors.size() - 1;
    // Mansfield Cox deBoor recursion

    if ((_i==0 && _t== m_knotVectors[0]) || (_i==m-_n-1 && _t==m_knotVectors[m]))
        return 1.0;

    if (_n == 0){
        if (_t >= m_knotVectors[_i] && _t < m_knotVectors[_i+1])
            return 1.0;
        else
            return 0.0;
    }

    double Nin1 = basisFunction(_i,   _n-1, _t);
    double Nin2 = basisFunction(_i+1, _n-1, _t);

    double fac1 = 0;
    if ((m_knotVectors[_i+_n]-m_knotVectors[_i]) !=0 )
        fac1 = (_t - m_knotVectors[_i]) / (m_knotVectors[_i+_n] - m_knotVectors[_i]) ;

    double fac2 = 0;
    if ( (m_knotVectors[_i+1+_n]-m_knotVectors[_i+1]) !=0 )
        fac2 = (m_knotVectors[_i+1+_n] - _t)/ (m_knotVectors[_i+1+_n] - m_knotVectors[_i+1]);

    return (fac1*Nin1 + fac2*Nin2);
}

float  BSpline::derivativeBasisFunction(int _i, int _n, double _t, int _der)
{
    if (_der == 0)
        return basisFunction(_i, _n, _t);

    double Nin1 = derivativeBasisFunction(_i,   _n-1, _t, _der-1);
    double Nin2 = derivativeBasisFunction(_i+1, _n-1, _t, _der-1);

    double fac1 = 0;
    if ( (m_knotVectors[_i+_n]-m_knotVectors[_i]) !=0 )
        fac1 = double(_n) / (m_knotVectors[_i+_n]-m_knotVectors[_i]);

    double fac2 = 0;
    if ( m_knotVectors[_i+1+_n]-m_knotVectors[_i+1] !=0 )
        fac2 = double(_n) / (m_knotVectors[_i+1+_n]-m_knotVectors[_i+1]);

    return (fac1*Nin1 - fac2*Nin2);
}

QPointF BSpline::inward_normal(int index)
{
    float range = (knotVectors()[knotVectors().size()-degree()-1] - knotVectors()[degree()]);
    float t = range*index / (float)(count()-1);
    QPointF tangent = derivativeCurvePoint(t, 1);

    if (index == count()-1)
    {
        if (connected_cpts[index] == connected_cpts[0])
            tangent = derivativeCurvePoint(0.0, 1);
        else
            tangent = pointAt(index) - pointAt(index-1);
    }

    float norm = sqrt(tangent.x()*tangent.x() + tangent.y()*tangent.y());
    if (norm > 1e-5)
        tangent /= norm;
    QPointF normal(-tangent.y(), tangent.x());
    //qDebug("%d: %.2f (%.2f %.2f) %.2f ", index, t, tangent.x(), tangent.y(), norm);

    return normal;
}
