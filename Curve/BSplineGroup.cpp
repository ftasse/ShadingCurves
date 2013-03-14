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

int BSplineGroup::addSurface(int splineRef, NormalDirection direction)
{
    Surface surface;
    surface.m_splineGroup = this;
    surface.ref = num_surfaces();
    surface.splineRef = splineRef;
    surface.direction = direction;
    m_surfaces.push_back(surface);
    spline(splineRef).surfaceRefs.push_back(surface.ref);
    return surface.ref;
}

bool BSplineGroup::addControlPointToSpline(int spline_id, int cpt_id)
{
    BSpline& bspline = spline(spline_id);
    if (bspline.has_loop()) return false;

    if (bspline.num_cpts() > 0 && bspline.cptRefs[0]!=cpt_id)
    {
        ControlPoint& front = bspline.pointAt(0);
        ControlPoint& cur = controlPoint(cpt_id);
        float dist = sqrt(pow(front.x()-cur.x(), 2.0) + pow(front.y()-cur.y(), 2.0));
        if (dist < 2.0)
        {
            cpt_id = front.ref;
        }
    }

    if (controlPoint(cpt_id).num_splines() > 0 && controlPoint(cpt_id).splineRefs.front()!=spline_id)    //
        splitCurveAt(controlPoint(cpt_id).splineRefs.front(), cpt_id);
    m_splines[spline_id].cptRefs.push_back(cpt_id);
    m_cpts[cpt_id].splineRefs.push_back(spline_id);

    for (int k=0; k<m_cpts[cpt_id].num_splines(); ++k)
    {
        m_splines[m_cpts[cpt_id].splineRefs[k]].recompute();
    }
    return true;
}

int BSplineGroup::splitCurveAt(int splineRef, int cptRef)
{
    BSpline& bspline = spline(splineRef);
    bool has_uniform_subdivision = bspline.has_uniform_subdivision;
    bool has_loop = bspline.has_loop();

    if ((!has_loop) && (bspline.cptRefs.front() == cptRef || bspline.cptRefs.back() == cptRef))
        return splineRef;

    int cptPos = -1;
    for (int k=0; k<bspline.num_cpts(); ++k)
    {
        if (bspline.cptRefs[k] == cptRef)
        {
            cptPos = k;
            break;
        }
    }
    if (cptPos < 0)
        return splineRef;

    //FLORA: Is this correct? We are moving the start of the control points in the spline
    //We probably just need to create new points differently for uniform subdivision
    if (has_loop && cptPos!=0 && bspline.pointAt(0).num_splines() <= 3)
    {
        QVector<int> new_cpts_refs;
        new_cpts_refs.push_back(bspline.cptRefs[cptPos]);
        int k = cptPos;
        do
        {
            int prev_k = k;
            k = (k+1)%bspline.num_cpts();
            if (bspline.cptRefs[k] != bspline.cptRefs[prev_k])
                new_cpts_refs.push_back(bspline.cptRefs[k]);
        } while (bspline.cptRefs[(k+1)%bspline.num_cpts()] != cptRef);
        new_cpts_refs.push_back(new_cpts_refs.front());

        bspline.cptRefs = new_cpts_refs;
        //if (has_uniform_subdivision)
        //    bspline.has_uniform_subdivision = false;
        bspline.recompute();
        //return splineRef;

        cptPos = -1;
        for (int k=0; k<bspline.num_cpts(); ++k)
        {
            if (bspline.cptRefs[k] == cptRef)
            {
                cptPos = k;
                break;
            }
        }
    }

    //Change position of the junction control point to the the limit point
    QVector<ControlPoint> points = bspline.getControlPoints();
    ControlPoint& junction = bspline.pointAt(cptPos);
    ControlPoint junctionCoords = junction;
    QPointF limitPoint;
    int left = cptPos>0?cptPos-1:points.size()-1;
    limitPoint= 0.1667*points[left]+0.667*points[cptPos]+0.1667*points[(cptPos+1)%points.size()];
    junction.setX(limitPoint.x());
    junction.setY(limitPoint.y());

    //if (has_loop)  return bspline.ref;

    //Create a new curve and move control points on the right to this new spline
    int newSplineRef = bspline.ref;
    if (cptPos != 0 && cptPos != bspline.num_cpts()-1)
    {
        newSplineRef = addBSpline();
    }
    BSpline& newSpline = spline(newSplineRef);
    if (newSplineRef != splineRef)
    {
        if (has_loop && has_uniform_subdivision)
        {
            ControlPoint& start = bspline.pointAt(0);
            limitPoint = 0.1667*points.last()+0.667*points[0]+0.1667*points[(1)%points.size()];
            start.setX(limitPoint.x());
            start.setY(limitPoint.y());
        }
        junction.splineRefs.push_back(newSplineRef);
        newSpline.cptRefs.push_back(junction.ref);
        while (cptPos+1 < bspline.num_cpts())
        {
            ControlPoint& cpt = bspline.pointAt(cptPos+1);
            newSpline.cptRefs.push_back(cpt.ref);
            for (int k=0; k<cpt.num_splines(); ++k)
            {
                if (cpt.splineRefs[k] == splineRef)
                {
                    cpt.splineRefs[k] = newSplineRef;
                }
            }
            bspline.cptRefs.erase(bspline.cptRefs.begin() + cptPos+1);
        }
    }

    //Modify the second control points for each curve from the junction
    int spline1_size = bspline.num_cpts();
    int spline2_size = newSpline.num_cpts();
    if (spline1_size > 1)
    {
        ControlPoint secondPoint = bspline.pointAt(bspline.num_cpts()-2);

        ControlPoint& cpt1 = bspline.pointAt(bspline.num_cpts()-2);
        ControlPoint newPosition1 = 0.33*secondPoint+0.667*junctionCoords;

        if (spline1_size == 2)
        {
           int newCptRef = addControlPoint(newPosition1, secondPoint.z());
           ControlPoint& newCpt = controlPoint(newCptRef);
           for (int k=0; k<2; ++k)
               newCpt.attributes[k]  = newCpt.attributes[k];
           newCpt.splineRefs.push_back(bspline.ref);
           bspline.cptRefs.insert(bspline.cptRefs.begin()+1, newCptRef);
        } else
        {
            cpt1.setX(newPosition1.x());
            cpt1.setY(newPosition1.y());
        }

        if (spline1_size > 2)
        {
            ControlPoint newPosition2 = 0.667*secondPoint+0.33*bspline.pointAt(bspline.num_cpts()-3);
            int newCptRef = addControlPoint(newPosition2, secondPoint.z());
            ControlPoint& cpt2 = controlPoint(newCptRef);
            for (int k=0; k<2; ++k)
                cpt2.attributes[k]  = cpt1.attributes[k];
            cpt2.splineRefs.push_back(bspline.ref);
            bspline.cptRefs.insert(bspline.cptRefs.begin()+bspline.num_cpts()-2, newCptRef);
        }
    }
    if (spline2_size > 1)
    {
        ControlPoint secondPoint = newSpline.pointAt(1);

        ControlPoint& cpt1 = newSpline.pointAt(1);
        QPointF newPosition1 = 0.33*secondPoint+0.667*junctionCoords;

        if (spline2_size == 2)
        {
           int newCptRef = addControlPoint(newPosition1, secondPoint.z());
           ControlPoint& newCpt = controlPoint(newCptRef);
           for (int k=0; k<2; ++k)
               newCpt.attributes[k]  = newCpt.attributes[k];
           newCpt.splineRefs.push_back(newSpline.ref);
           newSpline.cptRefs.insert(newSpline.cptRefs.begin()+1, newCptRef);
        } else
        {
            cpt1.setX(newPosition1.x());
            cpt1.setY(newPosition1.y());
        }

        if (spline2_size > 2)
        {
            ControlPoint newPosition2 = 0.667*secondPoint+0.33*newSpline.pointAt(2);
            int newCptRef = addControlPoint(newPosition2, secondPoint.z());
            ControlPoint& cpt2 = controlPoint(newCptRef);
            for (int k=0; k<2; ++k)
                cpt2.attributes[k]  = cpt1.attributes[k];
            cpt2.splineRefs.push_back(newSpline.ref);
            newSpline.cptRefs.insert(newSpline.cptRefs.begin()+2, newCptRef);
        }
    }

    if (has_loop && (bspline.ref != newSpline.ref))
    {
        bspline.has_uniform_subdivision = false;
        newSpline.has_uniform_subdivision = false;
    }

    return newSplineRef;
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

    while (bspline.surfaceRefs.size() > 0)
    {
        Surface& surf = surface(bspline.surfaceRefs[0]);
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
        if (bspline.surfaceAt(i).ref == surf.ref)
        {
            bspline.surfaceRefs.erase(bspline.surfaceRefs.begin()+i);
        } else
        {
            ++i;
        }
    }

    surf.vertices.clear();
    surf.controlMesh.clear();
    surf.sharpCorners.clear();
}

void BSplineGroup::scale(float xs, float ys)
{
    for (int i=0; i<num_controlPoints(); ++i)
    {
        controlPoint(i).setX(xs*controlPoint(i).x());
        controlPoint(i).setY(ys*controlPoint(i).y());
    }

    for (int i=0; i<colorMapping.size(); ++i)
    {

        QPoint prev = colorMapping[i].first;
        float nx = xs*prev.x();
        float ny = ys*prev.y();
        colorMapping[i].first = QPointF(nx, ny).toPoint();
    }

    for (int i=0; i<num_splines(); ++i)
    {
        spline(i).recompute();
    }
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
