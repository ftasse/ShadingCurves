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

BSpline::BSpline():
    ref(-1), has_inward_surface(false), has_outward_surface(false), has_uniform_subdivision(false), is_slope(false), generic_extent(30.0f)
{
}

void BSpline::change_generic_extent(float extent)
{
    generic_extent = extent;
    for (int i=0; i< num_cpts(); ++i)
    {
        pointAt(i).attributes[0].extent = generic_extent;
        pointAt(i).attributes[1].extent = generic_extent;
    }
}

void BSpline::change_bspline_type(bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward)
{
    is_slope = _is_slope;
    has_uniform_subdivision = _has_uniform_subdivision;
    has_inward_surface = _has_inward;
    has_outward_surface = _has_outward;
    recompute();
}

ControlPoint& BSpline::pointAt(int index)
{
    int cpt_idx = cptRefs[index];
    return m_splineGroup->controlPoint(cpt_idx);
}

Surface& BSpline::surfaceAt(int index)
{
    int surfRef = surfaceRefs[index];
    return m_splineGroup->surface(surfRef);
}

QPointF BSpline::inward_normal(int index, bool subdivided)
{
    if (subdivided)
    {
        return getNormal(getPoints(), index);
    } else
    {
        return getNormal(getControlPoints(), index);
    }
}

void BSpline::recompute()
{
    QVector<QPointF> points = getControlPoints();
    subdivided_points.clear();

    if (points.size() > 1)
        subdivided_points = subDivide(points, has_uniform_subdivision);

    if (has_uniform_subdivision && points.size() >= 4) {
        subdivided_points.pop_back();
        subdivided_points.pop_front();
    }
}

void BSpline::computeSurfaces(cv::Mat dt)
{
    //Call code for recomputing surfaces here
    while (num_surfaces() > 0)
    {
        m_splineGroup->removeSurface(surfaceRefs[0]);
    }

    /*if (has_inward_surface)
        m_splineGroup->createSurface(ref, dt, generic_extent, true);

    if (has_outward_surface)
        m_splineGroup->createSurface(ref, dt, generic_extent, false);*/
}

void BSpline::fix_orientation()
{
    if (cptRefs.size()>0 && has_loop())
    {
        //Check if current orientation is correct
        QPointF inside_point = getPoints().front() + 5*inward_normal(0, true);
        std::vector<cv::Point> contour;
        for (int i=0; i<getPoints().size(); ++i)
        {
            contour.push_back(cv::Point(getPoints()[i].x(), getPoints()[i].y()));
        }
        if (cv::pointPolygonTest(contour, cv::Point2f(inside_point.x(), inside_point.y()), false) < 0)
        {
            std::reverse(getPoints().begin(), getPoints().end());
        }
    }
}

QVector<QPointF> BSpline::getControlPoints()
{
    QVector<QPointF> points;
    for (int i=0; i< cptRefs.size(); ++i)
    {
        points.push_back(m_splineGroup->controlPoint(cptRefs[i]));
    }
    return points;
}

QVector<QPointF> BSpline::getPoints()
{
    if (subdivided_points.size() == 0)
    {
        recompute();
    }
    return subdivided_points;
}
