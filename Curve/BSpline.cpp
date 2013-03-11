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
    QVector<ControlPoint> points = getControlPoints();
    subdivided_points.clear();

    if (points.size() <=1)  return;
    //TODO Remove this right away
    for (int i=0; i<points.size(); ++i)
    {
        points[i].setZ(i*20+20);
        points[i].attributes[0].extent = 5*i+20;
        //points[i].print();
    }
    //printf("*****************************************\n");

    if (points.size() > 1)
        subdivided_points = subDivide(points, 2, has_uniform_subdivision);

    if (has_uniform_subdivision && points.size() >= 4) {
        subdivided_points.pop_back();
        subdivided_points.pop_front();
    }

    /*for (int i = 0; i<subdivided_points.size(); ++i)
    {
        subdivided_points[i].print();
    }*/
}

void BSpline::computeSurfaces(cv::Mat dt)
{
    bool recomputed_inward_surface = false;
    bool recomputed_outward_surface = false;

    //FLORA: recompute or delete surfaces
    for (int k=0; k<num_surfaces();)
    {
        if (surfaceAt(k).direction == INWARD_DIRECTION)
        {
            if (has_inward_surface)
            {
                surfaceAt(k).recompute(dt);
                recomputed_inward_surface = true;
                ++k;
            }
            else
            {
                m_splineGroup->removeSurface(surfaceRefs[k]);
            }
        } else if (surfaceAt(k).direction == OUTWARD_DIRECTION)
        {
            if (!is_slope  && has_outward_surface)
            {
                surfaceAt(k).recompute(dt);
                recomputed_outward_surface = true;
                ++k;
            } else
            {
                m_splineGroup->removeSurface(surfaceRefs[k]);
            }
        }
    }

    //FLORA: Create new surfaces if needed

    if (has_inward_surface  && !recomputed_inward_surface)
    {
        int surf_id = m_splineGroup->addSurface(ref, INWARD_DIRECTION);
        m_splineGroup->surface(surf_id).recompute(dt);
    }

    if (!is_slope && has_outward_surface && !recomputed_outward_surface)
    {
        int surf_id = m_splineGroup->addSurface(ref, OUTWARD_DIRECTION);
        m_splineGroup->surface(surf_id).recompute(dt);
    }
}

void BSpline::fix_orientation()
{
    if (cptRefs.size()>0 && has_loop())
    {
        //Check if current orientation is correct
        QPointF inside_point = (QPointF)getPoints().front() + 5*inward_normal(0, true);
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

QVector<ControlPoint> BSpline::getControlPoints()
{
    QVector<ControlPoint> points;
    for (int i=0; i< cptRefs.size(); ++i)
    {
        points.push_back(pointAt(i));
    }
    if (has_uniform_subdivision && has_loop() && points.size()>3)
    {
        points.push_back(points[1]);
        points.push_back(points[2]);
    }
    return points;
}

QVector<ControlPoint> BSpline::getPoints()
{
    if (subdivided_points.size() == 0)
    {
        recompute();
    }
    return subdivided_points;
}
