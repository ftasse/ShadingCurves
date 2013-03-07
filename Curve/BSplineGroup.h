#ifndef BSPLINEGROUP_H
#define BSPLINEGROUP_H

#include <QList>
#include "../Curve/ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/Surface.h"
#include "../Utilities/ImageUtils.h"

class BSplineGroup
{
public:
    BSplineGroup();

    int addControlPoint(QPointF value, float z=0.0);
    int addBSpline();
    int addSurface();

    bool addControlPointToSpline(int spline_id, int cpt_id);

    void removeControlPoint(int cpt_id);
    void removeSpline(int spline_id);
    void removeSurface(int surface_id);

    void garbage_collection();

    //IO
    bool load(std::string fname);
    void save(std::string fname);
    void saveOFF(std::string fname);

    //Utilities
    QList<Surface>& surfaces()
    {
        return m_surfaces;
    }

    QList<BSpline>& splines()
    {
        return m_splines;
    }

    QList<ControlPoint>& controlPoints()
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
    std::vector< std::pair<QPoint, QColor> > colorMapping;

private:
    QList<Surface> m_surfaces;
    QList<BSpline> m_splines;
    QList<ControlPoint> m_cpts;

    float EPSILON;
    float angleT;
    bool runningGarbageCollection;
};

#endif // BSPLINEGROUP_H
