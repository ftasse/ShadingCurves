#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include <QPainterPath>
#include <opencv2/core/core.hpp>

#include "ControlPoint.h"


class BSplineGroup;
class Surface;

class BSpline
{
public:
    BSpline();
    void recompute();
    void computeSurfaces(cv::Mat dt);
    void fix_orientation();
    void change_generic_extent(float extent);
    void change_bspline_type(bool _is_slope, bool _is_closed, bool _has_inward, bool _has_outward);

    //Normal at the (index)th control point
    QPointF inward_normal(int index, bool subdivided = true);

    //Utilites
    QVector<QPointF> getPoints(); // HENRIK: return list of control points
    QVector<QPointF> getControlPoints();
    ControlPoint& pointAt(int index);
    Surface& surfaceAt(int index);


    bool has_cycle()
    {
        return cptRefs.front() == cptRefs.back();
    }

    int num_cpts()
    {
        return cptRefs.size();
    }

    int num_surfaces()
    {
        return surfaceRefs.size();
    }

public:
    BSplineGroup *m_splineGroup;
    QVector<int> cptRefs;
    QVector<int> surfaceRefs;
    int ref;

    float generic_extent;
    bool is_slope;
    bool is_closed;
    bool has_inward_surface;
    bool has_outward_surface;

    QVector<QPointF> subdivided_points;
};

#endif // BSPLINE_H
