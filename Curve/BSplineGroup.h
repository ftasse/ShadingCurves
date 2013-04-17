#ifndef BSPLINEGROUP_H
#define BSPLINEGROUP_H

#include <QList>
#include "../Curve/ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/Surface.h"
#include "../Utilities/ImageUtils.h"

typedef struct CurveJunctionInfo
{
    int cptRef;
    int splineRef1;
    int splineRef2;
    int spline1Direction;
    int spline2Direction;
    bool has_negative_directions;
    bool valid;
    float height;

    CurveJunctionInfo()
    {
        cptRef = splineRef1 = splineRef2 = -1;
        has_negative_directions = valid = false;
        spline1Direction = spline2Direction = -1;
        height = 0.0;
    }
} CurveJunctionInfo;

class BSplineGroup
{
public:
    BSplineGroup();

    int addControlPoint(QPointF value, float z=0.0);
    int addBSpline();
    int addSurface(int splineRef, NormalDirection direction = INWARD_DIRECTION);

    bool addControlPointToSpline(int spline_id, int cpt_id);

    //FLORA: Return the handle for the new curve
    int splitCurveAt(int splineRef, int cptRef);

    void removeControlPoint(int cpt_id);
    void removeSpline(int spline_id);
    void removeSurface(int surface_id);

    void computeJunctions();

    void scale(float xs, float ys);
    void garbage_collection(bool keepOldIds = false);

    //IO
    bool load(std::string fname);
    void save(std::string fname);
    void saveOFF(std::string fname);

    void saveAll(std::string fname);
    void loadAll(std::string fname);

    //Utilities
    QVector<Surface>& surfaces()
    {
        return m_surfaces;
    }

    QVector<BSpline>& splines()
    {
        return m_splines;
    }

    QVector<ControlPoint>& controlPoints()
    {
        return m_cpts;
    }

    Surface& surface(int index)
    {
        return m_surfaces[index];
    }

    BSpline& spline(int index)
    {
        return m_splines[index];
    }

    ControlPoint& controlPoint(int index)
    {
        return m_cpts[index];
    }

    int num_surfaces()
    {
        return m_surfaces.size();
    }

    int num_splines()
    {
        return m_splines.size();
    }

    int num_controlPoints()
    {
        return m_cpts.size();
    }

public:
    cv::Size imageSize;
    std::vector< std::pair<QPoint, QColor> > colorMapping;
    QVector<CurveJunctionInfo> junctionInfos;

    std::map<int, int> new_cpt_indices;
    std::map<int, int> new_spline_indices;
    std::map<int, int> new_surface_indices;
private:
    QVector<Surface> m_surfaces;
    QVector<BSpline> m_splines;
    QVector<ControlPoint> m_cpts;

    float EPSILON;
    float angleT;
    bool runningGarbageCollection;
};

#endif // BSPLINEGROUP_H
