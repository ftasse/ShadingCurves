#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include <QPainterPath>
#include <opencv2/core/core.hpp>

#include "ControlPoint.h"

#define DEFAULT_SUBDV_LEVELS 1

class BSplineGroup;
class Surface;

class BSpline
{
public:
    BSpline();
    void recompute();
    void computeSurfaces(cv::Mat dt, cv::Mat luminance, bool clipHeight);
    void fix_orientation();
    void change_generic_extent(float extent);
    void change_bspline_type(bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward);

    //Normal at the (index)th control point
    QPointF get_normal(int index, bool subdivided = true, bool is_inward = true);

    //Utilites
    QVector<ControlPoint> getPoints(); // HENRIK: return list of control points
    QVector<ControlPoint> getDisplayPoints(int levels = 5, bool recompute = false);
    QVector<ControlPoint> getControlPoints();
    void computeControlPointNormals();
    void computeJunctionNormals(QVector<ControlPoint>& cpts, int i, QPointF& in_normal, QPointF& out_normal);

    std::string ghostSurfaceString(NormalDirection direction, cv::Mat img);

    ControlPoint& pointAt(int index);
    Surface& surfaceAt(int index);


    bool has_loop()
    {
        return (num_cpts()>1 && cptRefs.front() == cptRefs.back());
    }

    int num_cpts()
    {
        return cptRefs.size();
    }

    int num_surfaces()
    {
        return surfaceRefs.size();
    }

    QVector<QPointF> getNormals(bool is_inward)
    {
        if (inward_subdivided_normals.size() == 0)
            computeControlPointNormals();
        if (is_inward)  return inward_subdivided_normals;
        else    return outward_subdivided_normals;
    }

    void write(cv::FileStorage& fs) const ;
    void read(const cv::FileNode& node);

    BSpline(cv::FileNode node):ref(-1), has_inward_surface(false), has_outward_surface(false), has_uniform_subdivision(false), is_slope(false), generic_extent(30.0f)//:BSpline()
    {
        thickness = 0;
        read(node);
    }

public:
    BSplineGroup *m_splineGroup;
    QVector<int> cptRefs;
    QVector<int> surfaceRefs;
    int ref;

    float generic_extent;
    bool is_slope;
    bool has_uniform_subdivision;
    bool has_inward_surface;
    bool has_outward_surface;
    int thickness;
    int subv_levels;

    QVector<ControlPoint> subdivided_points;
    QVector<ControlPoint> display_points;
    QVector<QPointF> inward_subdivided_normals, outward_subdivided_normals;
    QVector<QPointF> inward_normals, outward_normals;

    bool start_has_zero_height[2];    //for inward and outward directions. Use for surface creation
    bool end_has_zero_height[2];
    std::vector< std::pair<int, float> > junctionPoints[2]; //Position and height
};

#endif // BSPLINE_H
