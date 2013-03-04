#include "BSpline.h"
#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"
#include <cmath>

float distance_sqr(QPointF a, QPointF b)
{
    QPointF vec = a-b;
    return vec.x()*vec.x()+vec.y()*vec.y();
}

float distance(QPointF a, QPointF b)
{
    return sqrt(distance_sqr(a, b));
}

QPointF nearestPoint(QPointF pt, QPointF a, QPointF b, float &t)
{
    QPointF ap = pt - a;
    QPointF ab = b - a;
    float ab2 = ab.x()*ab.x() + ab.y()*ab.y();
    float ap_ab = ap.x()*ab.x() + ap.y()*ab.y();
    t = ap_ab / ab2;
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    return a + ab * t;
}

BSpline::BSpline(int degree):
    idx(-1), m_spec_degree(degree), m_degree(degree), has_inward_surface(false), has_outward_surface(false)
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

float BSpline::closestParamToPointAt(int index)
{
    float lower = knotVectors()[degree()];
    float upper = knotVectors()[knotVectors().size()-degree()-1];
    QPointF pt = pointAt(index);

    //Todo Flora: This is slow. use a more sophisticated algorithm
    float t = 0.0;
    float dist = 1e8;
    int res = 20;
    for (int i=1; i<res; ++i)
    {
        float t1 = i*(upper-lower)/(res-1);
        float t2 = (i-1)*(upper-lower)/(res-1);
        float s;
        QPointF nearest =  nearestPoint(pt, curvePoint(t1), curvePoint(t2), s);
        float d_tmp = distance(pt, nearest);
        if (d_tmp < dist)
        {
            dist = d_tmp;
            t = t1*(1.0-s) + t2*s;
        }
    }

    return t;
}

QPointF BSpline::inward_normal_inaccurate(int index)
{
    QPointF tangent;

    //Use control line segments derivative
    if (index > 0 && index < count()-1) tangent = pointAt(index+1) - pointAt(index-1);
    else if (pointAt(0)==pointAt(count()-1)) tangent = pointAt(1) - pointAt(count()-2); // junction point
    else if (index == 0)    tangent = pointAt(index+1) - pointAt(index);
    else    tangent = pointAt(index) - pointAt(index-1);

    float norm = sqrt(tangent.x()*tangent.x() + tangent.y()*tangent.y());
    if (norm > 1e-5)
        tangent /= norm;
    QPointF normal(-tangent.y(), tangent.x());

    return normal;
}

QPointF BSpline::inward_normal(int index)
{
    float t = closestParamToPointAt(index);
    //Use spline derivatives
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

    return QPointF(-tangent.y(), tangent.x());
}

void BSpline::cleanup()
{
    if (connected_cpts.size() > 0)
    {
        while (connected_cpts.size()>0)
        {
            m_splineGroup->removeControlPoint(connected_cpts[0]);
        }
    }
}

void BSpline::recompute()
{
    cleanup();

    QVector<QPointF> points;
    for (int i=0; i< original_cpts.size(); ++i)
    {
        points.push_back(m_splineGroup->controlPoint(original_cpts[i]));
    }

    QVector<QPointF> subDividePts;

    if (points.size() > 1)
        subDividePts = subDivide(points);


    for (int i=0; i< subDividePts.size() -1 ; ++i)
    {
        int new_cpt = m_splineGroup->addControlPoint(subDividePts[i]);
        m_splineGroup->addControlPointToSpline(idx, new_cpt);
    }

    if (connected_cpts.size() >= 2)
    {
        if (is_closed())
        {
            m_splineGroup->addControlPointToSpline(idx, connected_cpts[0]);
        } else
        {
            int new_cpt = m_splineGroup->addControlPoint(subDividePts.back());
            m_splineGroup->addControlPointToSpline(idx, new_cpt);
        }
    }

    updateKnotVectors();
}

void BSpline::fix_orientation()
{
    if (connected_cpts.size()>0 && is_closed())
    {
        //Check if current orientation is correct
        QPointF inside_point = pointAt(0) + 5*inward_normal_inaccurate(0);
        std::vector<cv::Point> contour;
        for (int i=0; i<connected_cpts.size(); ++i)
        {
            contour.push_back(cv::Point(pointAt(i).x(), pointAt(i).y()));
        }
        if (cv::pointPolygonTest(contour, cv::Point2f(inside_point.x(), inside_point.y()), false) < 0)
            std::reverse(connected_cpts.begin(), connected_cpts.end());
    }
}

QVector<QPointF> BSpline::getPoints()
{
    QVector<QPointF> points;
    for (int i=0; i< connected_cpts.size(); ++i)
    {
        points.push_back(pointAt(i));
    }
    return points;
}
