#include <fstream>
#include <QDebug>
#include <QLineF>
#include <QColor>
#include <assert.h>
#include <stdio.h>
#include <set>

#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"

//These write and read functions must be defined for the serialization in FileStorage to work
static void write(cv::FileStorage& fs, const std::string&, const ControlPoint& x)
{
    x.write(fs);
}

static void read(const cv::FileNode& node, ControlPoint& x, const ControlPoint& default_value = ControlPoint()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

static void write(cv::FileStorage& fs, const std::string&, const BSpline& x)
{
    x.write(fs);
}

static void read(const cv::FileNode& node, BSpline& x, const BSpline& default_value = BSpline()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

typedef struct NormalInfo
{
    int pos;
    int direction;
    QPointF normal;
    NormalInfo (){}
    NormalInfo(int p, int d, QPointF n) {
        pos = p; direction = d; normal = n;
    }
} NormalInfo;

BSplineGroup::BSplineGroup()
{
    runningGarbageCollection = false;
}

int BSplineGroup::addControlPoint(QPointF value, float z)
{
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

    if (!controlPoint(cpt_id).isSharp && controlPoint(cpt_id).num_splines() > 0 && controlPoint(cpt_id).splineRefs.front()!=spline_id)
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

    if ((!has_loop) && (bspline.cptRefs.front() == cptRef || bspline.cptRefs.back() == cptRef)) return splineRef;

    int cptPos = -1;
    for (int k=0; k<bspline.num_cpts(); ++k)
    {
        if (bspline.cptRefs[k] == cptRef)
        {
            cptPos = k;
            break;
        }
    }
    if (cptPos < 0) return splineRef;

    if (has_loop && has_uniform_subdivision && cptPos!=0 && bspline.pointAt(0).num_splines() < 3)
    {
        for (int k=0; k<bspline.pointAt(0).num_splines(); ++k)
        {
            if (bspline.pointAt(0).splineRefs[k] == splineRef)
            {
                bspline.pointAt(0).splineRefs.erase(bspline.pointAt(0).splineRefs.begin() + k);
                break;
            }
        }
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
        bspline.pointAt(0).splineRefs.push_back(splineRef);
        bspline.recompute();

        cptPos = 0;
    }
    if (cptPos == 0 && !has_uniform_subdivision)
        return splineRef;

    //Change position of the junction control point to the the limit point
    QVector<ControlPoint> points = bspline.getControlPoints();
    ControlPoint& junction = bspline.pointAt(cptPos);
    ControlPoint junctionCoords = junction;
    QPointF limitPoint;
    int left = cptPos>0?cptPos-1:points.size()-1;   if (has_uniform_subdivision && cptPos==0)    left = bspline.num_cpts()-2;
    int right = (cptPos+1)%points.size();
    limitPoint= 0.1667*points[left]+0.667*points[cptPos]+0.1667*points[right];
    junction.setX(limitPoint.x());
    junction.setY(limitPoint.y());

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
                    break;
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
        ControlPoint newPosition1 = 0.33*secondPoint+0.667*junctionCoords;

        int newCptRef = addControlPoint(newPosition1);
        ControlPoint& newCpt = controlPoint(newCptRef);
        newCpt.splineRefs.push_back(bspline.ref);
        bspline.cptRefs.insert(bspline.cptRefs.begin()+bspline.num_cpts()-1, newCptRef);

    }
    if (spline2_size > 1)
    {
        ControlPoint secondPoint = newSpline.pointAt(1);
        QPointF newPosition1 = 0.33*secondPoint+0.667*junctionCoords;

        int newCptRef = addControlPoint(newPosition1);
        ControlPoint& newCpt = controlPoint(newCptRef);
        newCpt.splineRefs.push_back(newSpline.ref);
        newSpline.cptRefs.insert(newSpline.cptRefs.begin()+1, newCptRef);
    }

    if (has_loop)
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

    for (int i=0; i<junctionInfos.size();)
    {
        if (junctionInfos[i].cptRef == cpt_id)
            junctionInfos.erase(junctionInfos.begin() + i);
        else
            ++i;
    }
    cpt.splineRefs.clear();
    cpt.m_splineGroup = NULL;
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

    for (int i=0; i<junctionInfos.size();)
    {
        if (junctionInfos[i].splineRef1 == spline_id || junctionInfos[i].splineRef2 == spline_id)
            junctionInfos.erase(junctionInfos.begin() + i);
        else
            ++i;
    }

    bspline.cptRefs.clear();
    bspline.recompute();
    bspline.m_splineGroup = NULL;
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

    surf.faceIndices.clear();
    surf.vertices.clear();
    surf.controlMesh.clear();
    surf.sharpCorners.clear();
    surf.m_splineGroup = NULL;
}

void BSplineGroup::computeJunctions()
{
    junctionInfos.clear();
    std::vector< std::vector<bool> > bsplines_start_height_edited(num_splines()),
                                         bsplines_end_height_edited(num_splines());
    for (int i=0; i<num_splines(); ++i)
    {
        bsplines_start_height_edited[i] = std::vector<bool>(2,false);
        bsplines_end_height_edited[i] = std::vector<bool>(2,false);
    }

    for (int i=0; i<num_controlPoints(); ++i)
    {
        ControlPoint& cpt = controlPoint(i);
        if (cpt.num_splines() > 1)
        {
            for (int j=0; j<cpt.splineRefs.size(); ++j)
                for (int k=j+1; k<cpt.splineRefs.size(); ++k)
                {
                    if (cpt.splineRefs[j] == cpt.splineRefs[k])
                        continue;

                    BSpline& spline_j = spline(cpt.splineRefs[j]);
                    BSpline& spline_k = spline(cpt.splineRefs[k]);

                    if (spline_j.num_cpts()<=1 || spline_k.num_cpts()<=1 || spline_j.is_slope || spline_k.is_slope)
                        continue;

                    QVector< NormalInfo > normals_j;
                    QVector< NormalInfo > normals_k;

                    for (int l=0; l<spline_j.num_cpts(); ++l)
                        if (spline_j.cptRefs[l] == cpt.ref)
                        {
                            normals_j.push_back(NormalInfo(l, 0, spline_j.inward_normals[l]));
                            normals_j.push_back(NormalInfo(l, 1, spline_j.outward_normals[l]));
                        }

                    for (int l=0; l<spline_k.num_cpts(); ++l)
                        if (spline_k.cptRefs[l] == cpt.ref)
                        {
                            normals_k.push_back(NormalInfo(l, 0, spline_k.inward_normals[l]));
                            normals_k.push_back(NormalInfo(l, 1, spline_k.outward_normals[l]));
                        }

                    for (int m=0; m<normals_j.size(); ++m)
                        for (int n=0; n<normals_k.size(); ++n)
                        {
                            if (fabs(normals_j[m].normal.x()-normals_k[n].normal.x()) > 1e-8 ||
                                    fabs(normals_j[m].normal.y()-normals_k[n].normal.y()) > 1e-8)
                                continue;

                            if (normals_j[m].direction==0 && !spline_j.has_inward_surface)
                                continue;
                            else if (normals_j[m].direction==1 && !spline_j.has_outward_surface)
                                continue;
                            if (normals_k[n].direction==0 && !spline_k.has_inward_surface)
                                continue;
                            else if (normals_k[n].direction==1 && !spline_k.has_outward_surface)
                                continue;

                            CurveJunctionInfo junction;
                            junction.cptRef = cpt.ref;
                            junction.splineRef1 = spline_j.ref; junction.spline1Direction = normals_j[m].direction;
                            junction.splineRef2 = spline_k.ref; junction.spline2Direction = normals_k[n].direction;
                            junction.valid = true;
                            junction.has_negative_directions = false;

                            ControlPoint leftPt, rightPt;
                            if (normals_j[m].pos == 0)
                                leftPt = spline_j.pointAt(1);
                            else if (normals_j[m].pos == spline_j.num_cpts()-1)
                                leftPt = spline_j.pointAt(spline_j.num_cpts()-2);
                            else
                            {
                                qDebug("This is an invalid junction (1). cpt_id: %d", cpt.ref);
                                continue;
                            }

                            if (normals_k[n].pos == 0)
                                rightPt = spline_k.pointAt(1);
                            else if (normals_k[n].pos == spline_k.num_cpts()-1)
                                rightPt = spline_k.pointAt(spline_k.num_cpts()-2);
                            else
                            {
                                qDebug("This is an invalid junction (2). cpt_id: %d", cpt.ref);
                                continue;
                            }

                            junction.height - 0.0;
                            if (leftPt.attributes[junction.spline1Direction].height*rightPt.attributes[junction.spline2Direction].height<0.0)
                                junction.has_negative_directions = true;
                            else
                                junction.height = (leftPt.attributes[junction.spline1Direction].height + rightPt.attributes[junction.spline2Direction].height)/2.0;

                            spline_j.junctionPoints[junction.spline1Direction].push_back(std::pair<int, float>(normals_j[m].pos, junction.height));
                            spline_k.junctionPoints[junction.spline2Direction].push_back(std::pair<int, float>(normals_k[n].pos, junction.height));

                            junctionInfos.push_back(junction);

                            if (normals_j[m].pos == 0 || spline_j.has_loop())
                            {
                                if (!junction.has_negative_directions)
                                    bsplines_start_height_edited[spline_j.ref][junction.spline1Direction] = false;
                                if (!bsplines_start_height_edited[spline_j.ref][junction.spline1Direction])
                                {
                                    bsplines_start_height_edited[spline_j.ref][junction.spline1Direction] = true;
                                    spline_j.start_has_zero_height[junction.spline1Direction] = junction.has_negative_directions;
                                }
                            }
                            if (normals_j[m].pos == spline_j.num_cpts()-1 || spline_j.has_loop())
                            {
                                if (!junction.has_negative_directions)
                                    bsplines_end_height_edited[spline_j.ref][junction.spline1Direction] = false;
                                if (!bsplines_end_height_edited[spline_j.ref][junction.spline1Direction])
                                {
                                    bsplines_end_height_edited[spline_j.ref][junction.spline1Direction] = true;
                                    spline_j.end_has_zero_height[junction.spline1Direction] = junction.has_negative_directions;
                                }
                            }

                            if (normals_k[n].pos == 0 || spline_k.has_loop())
                            {
                                if (!junction.has_negative_directions)
                                    bsplines_start_height_edited[spline_k.ref][junction.spline2Direction] = false;
                                if (!bsplines_start_height_edited[spline_k.ref][junction.spline2Direction])
                                {
                                    bsplines_start_height_edited[spline_k.ref][junction.spline2Direction] = true;
                                    spline_k.start_has_zero_height[junction.spline2Direction] = junction.has_negative_directions;
                                }
                            }
                            if (normals_k[n].pos == spline_k.num_cpts()-1  || spline_k.has_loop())
                            {
                                if (!junction.has_negative_directions)
                                    bsplines_end_height_edited[spline_k.ref][junction.spline2Direction] = false;
                                if (!bsplines_end_height_edited[spline_k.ref][junction.spline2Direction])
                                {
                                    bsplines_end_height_edited[spline_k.ref][junction.spline2Direction] = true;
                                    spline_k.end_has_zero_height[junction.spline2Direction] = junction.has_negative_directions;
                                }
                            }

                        }
                }
        }
    }

    /*if (junctionInfos.size() > 0)
    {
        qDebug("nbr of junctions: %d", junctionInfos.size());
    }*/
}

void BSplineGroup::scale(float xs, float ys)
{
    for (int i=0; i<num_controlPoints(); ++i)
    {
        controlPoint(i).setX(xs*controlPoint(i).x());
        controlPoint(i).setY(ys*controlPoint(i).y());

        for (int k=0; k<2; ++k)
        {
            controlPoint(i).attributes[k].extent *= std::min(xs, ys);
        }
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

void BSplineGroup::garbage_collection(bool keepOldIds)
{
    if (runningGarbageCollection)
        return;
    else
        runningGarbageCollection = true;

    new_cpt_indices.clear();
    new_spline_indices.clear();
    new_surface_indices.clear();
    std::vector<int> remove_cpt_ids, remove_spline_ids, remove_surface_ids;

    junctionInfos.clear();

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
        m_surfaces.remove(remove_surface_ids[i]);
    }
    for (uint i=0; i< remove_spline_ids.size(); ++i)
    {
        m_splines.remove(remove_spline_ids[i]);
    }
    for (uint i=0; i< remove_cpt_ids.size(); ++i)
    {
        m_cpts.remove(remove_cpt_ids[i]);
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

    if (!keepOldIds)
    {
        new_cpt_indices.clear();
        new_spline_indices.clear();
        new_surface_indices.clear();
    }
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

void BSplineGroup::saveAll(std::string fname)
{
    garbage_collection();

    if (fname.find('.') == std::string::npos)
        fname += ".xml";

    cv::FileStorage fs(fname.c_str(), cv::FileStorage::WRITE);
    fs << "Resolution" << "{:" << "width" << imageSize.width << "height" << imageSize.height << "}";

    fs << "ControlPoints" << "[:";
    for (int i=0; i<num_controlPoints(); ++i)
    {
        fs << controlPoint(i);
    }
    fs << "]";

    fs << "Curves" << "[:";
    for (int i=0; i<num_splines(); ++i)
    {
        fs << spline(i);
    }
    fs << "]";

    fs << "ColorMappings" << "[:";
    for (int i=0; i<colorMapping.size(); ++i)
    {
        std::pair<QPoint, QColor> mapping = colorMapping[i];
        fs << "{:";
        fs << "point" << "{:" << "x" << mapping.first.x() << "y" << mapping.first.y() << "}";
        fs << "color" << "{:" << "red" << mapping.second.red() << "green" << mapping.second.green() << "blue" << mapping.second.blue() << "}";
        fs << "}";
    }
}

void BSplineGroup::loadAll(std::string fname)
{
    m_cpts.clear();
    m_splines.clear();
    m_surfaces.clear();
    colorMapping.clear();
    junctionInfos.clear();

    cv::FileStorage fs(fname.c_str(), cv::FileStorage::READ);
    fs["Resolution"]["width"] >> imageSize.width;
    fs["Resolution"]["height"] >> imageSize.height;

    cv::FileNode n = fs["ControlPoints"];
    {
        cv::FileNodeIterator it = n.begin(), it_end = n.end();
        for (; it != it_end; ++it)
        {
            controlPoints().push_back((ControlPoint)*it);
            controlPoints().last().m_splineGroup = this;
        }
    }

    n = fs["Curves"];
    {
        cv::FileNodeIterator it = n.begin(), it_end = n.end();
        for (; it != it_end; ++it)
        {
            splines().push_back((BSpline)*it);
            splines().last().m_splineGroup = this;
        }
    }

    n = fs["ColorMappings"];
    if (n.type() != cv::FileNode::EMPTY)
    {
        cv::FileNodeIterator it = n.begin(), it_end = n.end();
        for (; it != it_end; ++it)
        {
            QPoint point; QColor color;
            cv::FileNode pointNode = (*it)["point"], colorNode = (*it)["color"];
            if (pointNode.type() == cv::FileNode::NONE)
                continue;
            point.setX(pointNode["x"]); point.setY(pointNode["y"]);
            color.setRed(colorNode["red"]); color.setGreen(colorNode["green"]); color.setBlue(colorNode["blue"]);
            colorMapping.push_back(std::pair<QPoint, QColor>(point, color));
        }
    }
}
