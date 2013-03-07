#include <fstream>
#include <QDebug>
#include <QLineF>
#include <QColor>
#include <assert.h>
#include <stdio.h>
#include <set>

#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"

BSplineGroup::BSplineGroup()
{
    runningGarbageCollection = false;
}

int BSplineGroup::addControlPoint(QPointF value, float z)
{
    /*for (int i=0; i<num_controlPoints(); ++i)
    {
        if (!original && controlPoint(i).isOriginal)
            continue;
        float dx =  controlPoint(i).x() - value.x();
        float dy =  controlPoint(i).y() - value.y();
        float dz =  controlPoint(i).z() - z;
        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        if (fabs(dz) < EPSILON && dist < 5.0)
        {
            return i;
        } else if (fabs(dz) > EPSILON && dist < EPSILON)
        {
            return i;
        }
    }*/

    ControlPoint cpt(value);
    cpt.m_splineGroup = this;
    cpt.ref = num_controlPoints();
    cpt.setZ(z);
    m_cpts.push_back(cpt);
    return cpt.ref;
}

int BSplineGroup::addBSpline()
{
    BSpline spline;
    spline.m_splineGroup = this;
    spline.ref = num_splines();
    m_splines.push_back(spline);
    return m_splines.size() - 1;
}

int BSplineGroup::addSurface(NormalDirection direction)
{
    Surface surface;
    surface.m_splineGroup = this;
    surface.ref = num_surfaces();
    surface.direction = direction;
    m_surfaces.push_back(surface);
    return surface.ref;
}

bool BSplineGroup::addControlPointToSpline(int spline_id, int cpt_id)
{
    m_splines[spline_id].cptRefs.push_back(cpt_id);
    m_cpts[cpt_id].splineRefs.push_back(spline_id);
    m_splines[spline_id].recompute();
    return true;
}

void BSplineGroup::removeControlPoint(int cpt_id)
{
    ControlPoint& cpt = controlPoint(cpt_id);
    for (int i=0; i<cpt.splineRefs.size(); ++i)
    {
        BSpline& spline =  cpt.splineAt(i);
        for (int k=0; k<spline.num_cpts(); )
        {
            if (spline.cptRefs[k] == cpt_id)
            {
                spline.cptRefs.erase(spline.cptRefs.begin() + k);
            } else
            {
                ++k;
            }
        }
        spline.recompute();
    }
    cpt.splineRefs.clear();
}

void BSplineGroup::removeSpline(int spline_id)
{
    BSpline& bspline = spline(spline_id);
    for (int i=0; i<bspline.num_cpts(); ++i)
    {
        ControlPoint& cpt = bspline.pointAt(i);
        for (int k=0; k<cpt.num_splines(); ++k)
        {
            if (cpt.splineRefs[k] == spline_id)
            {
                cpt.splineRefs.erase(cpt.splineRefs.begin()+k);
            } else
            {
                 ++k;
            }
        }
    }

    for (int i=0; i<bspline.surfaceRefs.count(); ++i)
    {
        Surface& surf = surface(bspline.surfaceRefs[i]);
        removeSurface(surf.ref);
    }

    bspline.cptRefs.clear();
    bspline.recompute();
}

void BSplineGroup::removeSurface(int surface_id)
{
    Surface& surf = surface(surface_id);

    BSpline& bspline = spline(surf.splineRef);
    for (int i=0; i<bspline.num_surfaces();)
    {
        if (bspline.surfaceAt(i).ref = surf.ref)
        {
            bspline.surfaceRefs.erase(bspline.surfaceRefs.begin()+i);
        } else
        {
            ++i;
        }
    }

    surf.splineRef = -1;
    surf.vertices.clear();
    surf.controlMesh.clear();
    surf.sharpCorners.clear();
}

void BSplineGroup::garbage_collection()
{
    if (runningGarbageCollection)
        return;
    else
        runningGarbageCollection = true;

    std::map<int, int> new_cpt_indices;
    std::map<int, int> new_spline_indices;
    std::map<int, int> new_surface_indices;
    std::vector<int> remove_cpt_ids, remove_spline_ids, remove_surface_ids;

    for (int i=0; i<num_surfaces(); ++i)
    {
        if (surface(i).controlMesh.size() == 0)
        {
            remove_surface_ids.push_back(i-remove_surface_ids.size());
        } else
        {
        }
    }

    for (int i=0; i< num_splines(); ++i)
    {
        if (spline(i).cptRefs.size() == 0)
        {
            remove_spline_ids.push_back(i-remove_spline_ids.size());
        } else
        {
        }
    }

    for (int i=0; i< num_controlPoints(); ++i)
    {
        if (controlPoint(i).splineRefs.size() == 0)
        {
            remove_cpt_ids.push_back(i-remove_cpt_ids.size());
        }
    }

    for (uint i=0; i< remove_surface_ids.size(); ++i)
    {
        m_surfaces.removeAt(remove_surface_ids[i]);
    }
    for (uint i=0; i< remove_spline_ids.size(); ++i)
    {
        m_splines.removeAt(remove_spline_ids[i]);
    }
    for (uint i=0; i< remove_cpt_ids.size(); ++i)
    {
        m_cpts.removeAt(remove_cpt_ids[i]);
    }

    for (int i = 0; i< num_controlPoints(); ++i)
    {
        new_cpt_indices[controlPoint(i).ref] = i;
        controlPoint(i).ref = i;
    }
    for (int i = 0; i< num_splines(); ++i)
    {
        new_spline_indices[spline(i).ref] = i;
        spline(i).ref = i;
    }
    for (int i = 0; i< num_surfaces(); ++i)
    {
        new_surface_indices[surface(i).ref] = i;
        surface(i).ref = i;
    }

    for (int i = 0; i< num_controlPoints(); ++i)
    {
        ControlPoint& cpt = controlPoint(i);
        for (int k=0; k<cpt.splineRefs.size(); ++k)
        {
            cpt.splineRefs[k] = new_spline_indices[cpt.splineRefs[k]];
        }
    }

    for (int i = 0; i< num_splines(); ++i)
    {
        BSpline& bspline = spline(i);
        for (int k=0; k<bspline.cptRefs.size(); ++k)
        {
            bspline.cptRefs[k] = new_cpt_indices[bspline.cptRefs[k]];
        }

        for (int k=0; k<bspline.surfaceRefs.size(); ++k)
        {
            bspline.surfaceRefs[k] = new_surface_indices[bspline.surfaceRefs[k]];
        }

        if (bspline.cptRefs.size() > 0)
        {
            bspline.recompute();
        }
    }

    for (int i=0; i<num_surfaces(); ++i)
    {
        Surface& surf = surface(i);
        surf.splineRef = new_spline_indices[surf.splineRef];
    }

    runningGarbageCollection = false;
}


bool BSplineGroup::load(std::string fname)
{
    std::ifstream ifs(fname.c_str());
    if (!ifs.is_open())
        return false;

    m_cpts.clear();
    m_splines.clear();
    m_surfaces.clear();
    colorMapping.clear();

    int n_cpts, n_splines;
    std::string text;
    ifs >> n_cpts >> text;

    for (int i=0; i<n_cpts; ++i)
    {
        float _x, _y;
        ifs >> _x >> _y;
        addControlPoint(QPointF(_x, _y), 0.0);
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
        spline(spline_id).recompute();
    }

    int n_colormappings;
    ifs >> n_colormappings >> text;

    for (int i=0; i<n_colormappings; ++i)
    {
        int x, y, red, blue, green;
        ifs >> x >> y >> red >> green >> blue;

        QColor color;
        color.setRed(red); color.setGreen(green); color.setBlue(blue);
        colorMapping.push_back(std::pair<QPoint, QColor> (QPoint(x,y), color));
    }
    return true;
}

void BSplineGroup::save(std::string fname)
{
    garbage_collection();

    std::map<int, int> vertex_indices;

    int N=0;
    for (int i=0; i< num_controlPoints(); ++i)
    {
        vertex_indices[i] = N;
        ++N;
    }

    std::ofstream ofs(fname.c_str());
    ofs << N <<" points" << std::endl;
    for (std::map<int, int>::iterator it = vertex_indices.begin(); it != vertex_indices.end(); ++it)
    {
        int i = it->second;
        ofs << controlPoint(i).x() << " " << controlPoint(i).y() << std::endl;
    }
    ofs << num_splines() <<" splines" << std::endl;
    for (int i=0; i<num_splines(); ++i)
    {
        BSpline& bspline = spline(i);

        ofs << bspline.cptRefs.size();
        for (int k=0; k<bspline.cptRefs.size(); ++k)
        {
            ofs << " " << vertex_indices[bspline.cptRefs[k]];
        }
        ofs << std::endl;
    }

    ofs << colorMapping.size() <<" colorMapping" << std::endl;
    for (int i=0; i<colorMapping.size(); ++i)
    {
        QPoint point = colorMapping[i].first;
        QColor color = colorMapping[i].second;
        ofs << point.x() << " " << point.y() << " " << color.red() << " " << color.green() << " " << color.blue() << std::endl;
    }
    ofs.close();
}

// HENRIK: save CPs to OFF (TODO)
void BSplineGroup::saveOFF(std::string fname)
{
    garbage_collection();

    std::map<int, int> vertex_indices;

    int N=0;
    for (int i=0; i< num_controlPoints(); ++i)
    {
        vertex_indices[i] = N;
        ++N;
    }

    std::ofstream ofs(fname.c_str());
    ofs << N <<" points" << std::endl;
    for (std::map<int, int>::iterator it = vertex_indices.begin(); it != vertex_indices.end(); ++it)
    {
        int i = it->second;
        ofs << controlPoint(i).x() << " " << controlPoint(i).y() << std::endl;
    }
    ofs << num_splines() << " splines" << std::endl;
    for (int i=0; i<num_splines(); ++i)
    {
        BSpline& bspline = spline(i);

        ofs << bspline.cptRefs.size();
        for (int k=0; k<bspline.cptRefs.size(); ++k)
        {
            ofs << " " << vertex_indices[bspline.cptRefs[k]];
        }
        ofs << std::endl;
    }
    ofs.close();
}
