#include "../Curve/Surface.h"
#include "../Curve/BSplineGroup.h"

int createUniformKnotVectors(QVector<float>& knots, int spec_degree, int n)
{
    //if number of control points is less than spec_degree+1, return "n-1"
    //else return spec_degree

    knots.clear();
    int degree;
    if (n == 0)
        return 0;

    if (n > spec_degree)
        degree = spec_degree;
    else
         degree = n-1;

    float last=0.0;
    for (unsigned int i = 0; i < degree; i++) knots.push_back(0.0);
    for (int i = 0; i < n - degree + 1; i++) {
        knots.push_back(i);
        last=i;
    }
    for (unsigned int i = 0; i < degree; i++)
        knots.push_back(last);
    return degree;
}

Surface::Surface(QPoint degree):
    m_spec_degree(degree), idx(-1), connected_spline_id(-1)
{
}

void Surface::updateKnotVectors()
{
    if (controlPoints().size() > 0)
    {
        m_degree.setX( createUniformKnotVectors(m_knotVectors_u, m_spec_degree.x(), controlPoints().size()) );
        m_degree.setY( createUniformKnotVectors(m_knotVectors_v, m_spec_degree.y(), controlPoints()[0].size()) );
    } else
    {
        m_knotVectors_u.clear();
        m_knotVectors_v.clear();
        m_degree = QPoint(0, 0);
    }
}

ControlPoint& Surface::pointAt(QPoint pos)
{
    int cpt_idx = connected_cpts[pos.x()][pos.y()];
    return m_splineGroup->controlPoint(cpt_idx);
}
