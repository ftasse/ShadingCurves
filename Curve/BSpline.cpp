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

    if (count() > m_spec_degree)
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
