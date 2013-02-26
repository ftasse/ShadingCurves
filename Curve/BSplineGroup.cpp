#include <fstream>
#include <QDebug>
#include <QLineF>
#include <assert.h>
#include <stdio.h>
#include <set>

#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"

BSplineGroup::BSplineGroup()
{
    EPSILON = .0001f;
    runningGarbageCollection = false;
}

int BSplineGroup::addControlPoint(QPointF value, float z, bool original)
{
    /*for (int i=0; i<num_controlPoints(); ++i)
    {
        if (!original && controlPoint(i).isOriginal)
            continue;
        float dx =  controlPoint(i).x() - value.x();
        float dy =  controlPoint(i).y() - value.y();
        float dz =  controlPoint(i).z() - z;
        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        if (abs(dz) < EPSILON && dist < 5.0)
        {
            return i;
        } else if (abs(dz) > EPSILON && dist < EPSILON)
        {
            return i;
        }
    }*/

    ControlPoint cpt(value);
    cpt.m_splineGroup = this;
    cpt.idx = num_controlPoints();
    cpt.setZ(z);
    cpt.isOriginal = original;
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

int BSplineGroup::createSurface(int spline_id, cv::Mat dt, float width, bool inward)
{
    int z;
    float angleT = 35.0f;

    if (inward)
    {
        z = 100;
    }
    else
    {
        z = -100;
    }

    int surface_id = addSurface();
    Surface& surf = surface(surface_id);
    BSpline& bspline = spline(spline_id);

    bspline.fix_orientation();

    surf.connected_spline_id = spline_id;
    QVector<int> translated_cpts_ids;
    QVector<int> perpendicular_cpts_ids;

    for (int k=0; k<bspline.count(); ++k)
    {
        if (k == bspline.count()-1 && bspline.is_closed()) //if closed curve
        {
            perpendicular_cpts_ids.push_back(perpendicular_cpts_ids[0]);
        } else
        {
            QPointF new_cpt = bspline.pointAt(k);
            int cpt_id = addControlPoint(new_cpt, z);
            perpendicular_cpts_ids.push_back(cpt_id);
        }
    }

    // get limit points for the control points
    QVector<QPointF> lp = limitPoints(bspline.getPoints());

    // loop through all control points for the given spline curve
    for (int k=0; k<bspline.count(); ++k)
    {
        if (k == bspline.count()-1 && bspline.is_closed()) //if closed curve
        {
            translated_cpts_ids.push_back(translated_cpts_ids[0]);
        } else
        {
            // HENRIK: move in the distance transform image
            float currentD = 0;
            QPointF normal = bspline.inward_normal(k);
            if(!inward)
                normal = -normal;
            QLineF normalL(lp.at(k),lp.at(k) + normal*width);
            QPointF tmp = lp.at(k)+normal*2;
            QPoint current(qRound(tmp.x()),qRound(tmp.y()));
            QPointF new_cpt;
            QList<QPoint> visited;

            while(true) {
                float oldD = currentD;
                QPoint m = localMax(dt,cv::Rect(current.x()-2,current.y()-2,current.x()+2,current.y()+2)
                                    ,&currentD,normalL,visited);
                // check lines
                QLineF currentL(lp.at(k),m);
                float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
                if(abs(oldD-currentD)<EPSILON || currentD >= width || angle > angleT) {
                    new_cpt.rx() = m.rx();
                    new_cpt.ry() = m.ry();
                    break;
                } else {
                    visited.append(current);
                    current = m;
                }
            }

            int cpt_id = addControlPoint(new_cpt);
            translated_cpts_ids.push_back(cpt_id);
        }
    }

    surf.controlPoints().append(bspline.connected_cpts);
    surf.controlPoints().append(perpendicular_cpts_ids);
    surf.controlPoints().append(translated_cpts_ids);

    surf.updateKnotVectors();
    return surface_id;
}

bool BSplineGroup::addControlPointToSpline(int spline_id, int cpt_id, bool original)
{
    if (original)
        m_splines[spline_id].original_cpts.push_back(cpt_id);
    else
    {
        m_splines[spline_id].connected_cpts.push_back(cpt_id);
    }

    m_cpts[cpt_id].connected_splines.push_back(spline_id);

    if (original)
    {
        m_splines[spline_id].recompute();
    }
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
        for (int k=0; k<spline.original_cpts.size(); )
        {
            if (spline.original_cpts[k] == cpt_id)
            {
                spline.original_cpts.erase(spline.original_cpts.begin() + k);
            } else
            {
                ++k;
            }
        }
        if (cpt.isOriginal)
            spline.recompute();
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
    std::vector<bool> is_surface_point(num_controlPoints(), false);

    for (int i=0; i<num_surfaces(); ++i)
    {
        if (surface(i).connected_cpts.size() == 0)
        {
            remove_surface_ids.push_back(i-remove_surface_ids.size());
        } else
        {
            for (int k=0; k<surface(i).controlPoints().size(); ++k)
            {
                for (int l=0; l<surface(i).controlPoints()[k].size(); ++l)
                {
                    is_surface_point[surface(i).controlPoints()[k][l]] = true;
                }
            }
        }
    }

    for (int i=0; i< num_splines(); ++i)
    {
        spline(i).cleanup();
        if (spline(i).original_cpts.size() == 0)
        {
            remove_spline_ids.push_back(i-remove_spline_ids.size());
        } else
        {
        }
    }

    for (int i=0; i< num_controlPoints(); ++i)
    {
        if (controlPoint(i).connected_splines.size() == 0 && !is_surface_point[i])
        {
            remove_cpt_ids.push_back(i-remove_cpt_ids.size());
        } else
        {
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
        new_cpt_indices[controlPoint(i).idx] = i;
        controlPoint(i).idx = i;
    }
    for (int i = 0; i< num_splines(); ++i)
    {
        new_spline_indices[spline(i).idx] = i;
        spline(i).idx = i;
    }
    for (int i = 0; i< num_surfaces(); ++i)
    {
        new_surface_indices[surface(i).idx] = i;
        surface(i).idx = i;
    }

    for (int i = 0; i< num_controlPoints(); ++i)
    {
        ControlPoint& cpt = controlPoint(i);
        for (int k=0; k<cpt.connected_splines.size(); ++k)
        {
            cpt.connected_splines[k] = new_spline_indices[cpt.connected_splines[k]];
        }
    }

    for (int i = 0; i< num_splines(); ++i)
    {
        BSpline& bspline = spline(i);
        for (int k=0; k<bspline.original_cpts.size(); ++k)
        {
            bspline.original_cpts[k] = new_cpt_indices[bspline.original_cpts[k]];
        }

        if (bspline.original_cpts.size() > 0)
        {
            bspline.recompute();
        }
    }

    for (int i=0; i<num_surfaces(); ++i)
    {
        Surface& surf = surface(i);
        surf.connected_spline_id = new_spline_indices[surf.connected_spline_id];
        for (int k=0; k<surf.controlPoints().size(); ++k)
        {
                for (int l=0; l<surf.controlPoints()[k].size(); ++l)
                {
                    surf.controlPoints()[k][l] = new_cpt_indices[surf.controlPoints()[k][l]];;
                }
        }
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

    int n_cpts, n_splines;
    std::string text;
    ifs >> n_cpts >> text;

    for (int i=0; i<n_cpts; ++i)
    {
        float _x, _y;
        ifs >> _x >> _y;
        addControlPoint(QPointF(_x, _y), 0.0, true);
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
                addControlPointToSpline(spline_id, cpt_id, true);
            }
        }
        spline(spline_id).recompute();
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
        if (controlPoint(i).isOriginal)
        {
            vertex_indices[i] = N;
            ++N;
        }
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

        ofs << bspline.original_cpts.size();
        for (int k=0; k<bspline.original_cpts.size(); ++k)
        {
            ofs << " " << vertex_indices[bspline.original_cpts[k]];
        }
        ofs << std::endl;
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
        if (controlPoint(i).isOriginal)
        {
            vertex_indices[i] = N;
            ++N;
        }
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

        ofs << bspline.original_cpts.size();
        for (int k=0; k<bspline.original_cpts.size(); ++k)
        {
            ofs << " " << vertex_indices[bspline.original_cpts[k]];
        }
        ofs << std::endl;
    }
    ofs.close();
}


// HENRIK: find max value in I, in neighbourhood N
QPoint BSplineGroup::localMax(cv::Mat I,cv::Rect N,float* oldD,QLineF normalL,QList<QPoint> visited)
{
    int sx = N.x;
    int sy = N.y;
    cv::Size S = I.size();
    float m = *oldD;
    QList<QPoint> cand; // candidates
    for(int x=sx;x<=N.width;x++)
        for(int y=sy;y<=N.height;y++) {
            if(x<0 || x>=S.width || y<0 || y>=S.height)
                continue;
            float d = I.at<float>(y,x);
            bool visCheck = visited.contains(QPoint(x,y));
            if(abs(d-m)<EPSILON && !visCheck)
                cand.append(QPoint(x,y));
            else if(d>m) {
                m=d;
                cand.clear();
                cand.append(QPoint(x,y));
            }
            assert(!(d-m>EPSILON && visCheck));
        }

    // loop through the candidates
    float ma = 360; // min angle
    QPoint winner;
    for (QList<QPoint>::iterator it = cand.begin(); it!=cand.end(); it++) {
        QLineF currentL(normalL.p1(),*it);
        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        if(angle<ma) {
            ma = angle;
            winner = *it;
        }
    }
    *oldD = m;
    return winner;
}
