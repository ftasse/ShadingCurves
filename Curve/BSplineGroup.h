#ifndef BSPLINEGROUP_H
#define BSPLINEGROUP_H

#include <QList>
#include "../Curve/ControlPoint.h"
#include "../Curve/BSpline.h"

class BSplineGroup
{
public:
    BSplineGroup();

    int addControlPoint(QPointF value);
    int addBSpline();
    bool addControlPoint(int spline_id, int cpt_id);

    void removeControlPoint(int cpt_id);
    void removeSpline(int spline_id);

    //IO
    bool load(std::string fname);
    void save(std::string fname);

    //Utilities
    QList<BSpline>& splines()
    {
        return m_splines;
    }

    QList<ControlPoint>& controlPoints()
    {
        return m_cpts;
    }

    BSpline& spline(int index)
    {
        return m_splines[index];
    }

    ControlPoint& controlPoint(int index)
    {
        return m_cpts[index];
    }

    int num_splines()
    {
        return m_splines.size();
    }

    int num_controlPoints()
    {
        return m_cpts.size();
    }

private:
    QList<BSpline> m_splines;
    QList<ControlPoint> m_cpts;
};

#endif // BSPLINEGROUP_H
