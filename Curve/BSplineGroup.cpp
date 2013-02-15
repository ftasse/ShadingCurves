#include <fstream>

#include "BSplineGroup.h"

BSplineGroup::BSplineGroup()
{
}

int BSplineGroup::addControlPoint(QPointF value)
{
    ControlPoint cpt(value);
    cpt.m_splineGroup = this;
    cpt.idx = num_controlPoints();
    m_cpts.push_back(cpt);
    return m_cpts.size() - 1;
}

int BSplineGroup::addBSpline()
{
    BSpline spline;
    spline.m_splineGroup = this;
    spline.idx = num_splines();
    m_splines.push_back(spline);
    return m_splines.size() - 1;
}

int BSplineGroup::addSurface()
{
    Surface surface;
    surface.m_splineGroup = this;
    surface.idx = num_surfaces();
    m_surfaces.push_back(surface);
    return num_surfaces() - 1;
}

int BSplineGroup::createSurface(int spline_id, float width)
{
    int surface_id = addSurface();
    Surface& surf = surface(surface_id);
    BSpline& bspline = spline(spline_id);

    surf.connected_spline_id = spline_id;
    surf.controlPoints().append(bspline.connected_cpts);
    QVector<int> translated_cpts_ids;

    for (int k=0; k<bspline.count(); ++k)
    {
        if (k == bspline.count()-1 && bspline.connected_cpts[k] == bspline.connected_cpts[0]) //if closed curve
        {
            translated_cpts_ids.push_back(translated_cpts_ids[0]);
        } else
        {
            QPointF normal = bspline.inward_normal(k);
            QPointF new_cpt = bspline.pointAt(k) + normal*width;
            int cpt_id = addControlPoint(new_cpt);
            translated_cpts_ids.push_back(cpt_id);
        }
    }

    surf.controlPoints().append(translated_cpts_ids);

    surf.updateKnotVectors();
    return surface_id;
}

bool BSplineGroup::addControlPointToSpline(int spline_id, int cpt_id)
{
    m_splines[spline_id].connected_cpts.push_back(cpt_id);
    m_cpts[cpt_id].connected_splines.push_back(spline_id);
    m_splines[spline_id].updateKnotVectors();
    return true;
}

void BSplineGroup::removeControlPoint(int cpt_id)
{
    ControlPoint& cpt = controlPoint(cpt_id);
    for (int i=0; i<cpt.connected_splines.size(); ++i)
    {
        BSpline& spline =  cpt.splineAt(i);
        for (int k=0; k<spline.count(); )
        {
            if (spline.connected_cpts[k] == cpt_id)
            {
                spline.connected_cpts.erase(spline.connected_cpts.begin() + k);
            } else
            {
                ++k;
            }
        }
        spline.updateKnotVectors();
    }
    cpt.connected_splines.clear();
}

void BSplineGroup::removeSpline(int spline_id)
{
    BSpline& bspline = spline(spline_id);
    for (int i=0; i<bspline.count(); ++i)
    {
        ControlPoint& cpt = bspline.pointAt(i);
        for (int k=0; k<cpt.count();)
        {
            if (cpt.connected_splines[k] == spline_id)
            {
                cpt.connected_splines.erase(cpt.connected_splines.begin()+k);
            } else
            {
                 ++k;
            }
        }
    }


    for (int i=0; i<m_surfaces.size(); ++i)
    {
        Surface& surf = surface(i);
        if (surf.connected_spline_id == spline_id)
        {
            removeSurface(surf.idx);
        }
    }

    bspline.connected_cpts.clear();
    bspline.updateKnotVectors();
}

void BSplineGroup::removeSurface(int surface_id)
{
    Surface& surf = surface(surface_id);
    surf.connected_cpts.clear();
    surf.updateKnotVectors();
}


bool BSplineGroup::load(std::string fname)
{
    std::ifstream ifs(fname.c_str());
    if (!ifs.is_open())
        return false;

    int n_cpts, n_splines;
    std::string text;
    ifs >> n_cpts >> text;

    for (int i=0; i<n_cpts; ++i)
    {
        float _x, _y;
        ifs >> _x >> _y;
        addControlPoint(QPointF(_x, _y));
    }

    ifs >> n_splines >> text;
    for (int i=0; i<n_splines; ++i)
    {
        int count, spline_id;
        ifs >> count;

        if (count > 0)
        {
            spline_id = addBSpline();
            for (int k=0; k<count; ++k)
            {
                int cpt_id;
                ifs >> cpt_id;
                addControlPointToSpline(spline_id, cpt_id);
            }
        }
    }
    return true;
}

void BSplineGroup::save(std::string fname)
{
    std::ofstream ofs(fname.c_str());
    ofs << num_controlPoints() <<" points" << std::endl;
    for (int i=0; i<num_controlPoints(); ++i)
    {
        ofs << controlPoint(i).x() << " " << controlPoint(i).y() << std::endl;
    }
    ofs << num_splines() <<" splines" << std::endl;
    for (int i=0; i<num_splines(); ++i)
    {
        BSpline& bspline = spline(i);

        ofs << bspline.count();
        for (int k=0; k<bspline.count(); ++k)
        {
            ofs << " " << bspline.connected_cpts[k];
        }
        ofs << std::endl;
    }
    ofs.close();
}
