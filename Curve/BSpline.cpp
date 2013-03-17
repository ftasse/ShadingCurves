#include "BSpline.h"
#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"
#include <cmath>

float distance_sqr(QPointF a, QPointF b)
{
    QPointF vec = a-b;
    return vec.x()*vec.x()+vec.y()*vec.y();
}

float distance(QPointF a, QPointF b)
{
    return sqrt(distance_sqr(a, b));
}

QPointF unit(QPointF vec)
{
    float norm = sqrt(vec.x()*vec.x() + vec.y()*vec.y());
    if (norm > 1e-8)
    {
        vec /= norm;
    }
    return vec;
}

QPointF nearestPoint(QPointF pt, QPointF a, QPointF b, float &t)
{
    QPointF ap = pt - a;
    QPointF ab = b - a;
    float ab2 = ab.x()*ab.x() + ab.y()*ab.y();
    float ap_ab = ap.x()*ab.x() + ap.y()*ab.y();
    t = ap_ab / ab2;
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    return a + ab * t;
}

BSpline::BSpline():
    ref(-1), has_inward_surface(false), has_outward_surface(false), has_uniform_subdivision(false), is_slope(false), generic_extent(30.0f)
{
    thickness = 0;
}

void BSpline::change_generic_extent(float extent)
{
    generic_extent = extent;
    for (int i=0; i< num_cpts(); ++i)
    {
        pointAt(i).attributes[0].extent = generic_extent;
        pointAt(i).attributes[1].extent = generic_extent;
    }
}

void BSpline::change_bspline_type(bool _is_slope, bool _has_uniform_subdivision, bool _has_inward, bool _has_outward)
{
    is_slope = _is_slope;
    has_uniform_subdivision = _has_uniform_subdivision;
    has_inward_surface = _has_inward;
    has_outward_surface = _has_outward;
    recompute();
}

ControlPoint& BSpline::pointAt(int index)
{
    int cpt_idx = cptRefs[index];
    return m_splineGroup->controlPoint(cpt_idx);
}

Surface& BSpline::surfaceAt(int index)
{
    int surfRef = surfaceRefs[index];
    return m_splineGroup->surface(surfRef);
}

QPointF BSpline::get_normal(int index, bool subdivided, bool is_inward)
{
    if (inward_subdivided_normals.size() == 0 && cptRefs.size()>1)
    {
        computeControlPointNormals();
    }
    NormalDirection direction = is_inward?INWARD_DIRECTION:OUTWARD_DIRECTION;
    if (!subdivided)
    {
        if (direction == INWARD_DIRECTION)
            return inward_normals[index];
        else
            return outward_normals[index];
    } else
    {
        if (direction == INWARD_DIRECTION)
            return inward_subdivided_normals[index];
        else
            return outward_subdivided_normals[index];
    }
}

void BSpline::recompute()
{
    QVector<ControlPoint> points = getControlPoints();
    subdivided_points.clear();

    inward_normals.clear();
    outward_normals.clear();
    inward_subdivided_normals.clear();
    outward_subdivided_normals.clear();

    if (points.size() <=1)  return;
    //TODO Remove this right away
/*    for (int i=0; i<points.size(); ++i)
    {
//        points[i].setZ(i*20+20);
        points[i].attributes[0].extent = 60;
        points[i].attributes[1].extent = 60;
        //points[i].print();
    }*/
    //printf("*****************************************\n");

    if (points.size() > 1)
        subdivided_points = subDivide(points, 2, has_uniform_subdivision);

    if (has_uniform_subdivision && points.size() >= 4) {
        subdivided_points.pop_back();
        subdivided_points.pop_front();
    }

    /*for (int i = 0; i<subdivided_points.size(); ++i)
    {
        subdivided_points[i].print();
    }*/

}

void BSpline::computeSurfaces(cv::Mat dt)
{
    bool recomputed_inward_surface = false;
    bool recomputed_outward_surface = false;

    //FLORA: recompute or delete surfaces
    for (int k=0; k<num_surfaces();)
    {
        if (surfaceAt(k).direction == INWARD_DIRECTION)
        {
            if (has_inward_surface)
            {
                surfaceAt(k).recompute(dt);
                recomputed_inward_surface = true;
                ++k;
            }
            else
            {
                m_splineGroup->removeSurface(surfaceRefs[k]);
            }
        } else if (surfaceAt(k).direction == OUTWARD_DIRECTION)
        {
            if (!is_slope  && has_outward_surface)
            {
                surfaceAt(k).recompute(dt);
                recomputed_outward_surface = true;
                ++k;
            } else
            {
                m_splineGroup->removeSurface(surfaceRefs[k]);
            }
        }
    }

    //FLORA: Create new surfaces if needed

    if (has_inward_surface  && !recomputed_inward_surface)
    {
        int surf_id = m_splineGroup->addSurface(ref, INWARD_DIRECTION);
        m_splineGroup->surface(surf_id).recompute(dt);
    }

    if (!is_slope && has_outward_surface && !recomputed_outward_surface)
    {
        int surf_id = m_splineGroup->addSurface(ref, OUTWARD_DIRECTION);
        m_splineGroup->surface(surf_id).recompute(dt);
    }
}

QVector<ControlPoint> BSpline::getControlPoints()
{
    QVector<ControlPoint> points;
    for (int i=0; i< cptRefs.size(); ++i)
    {
        points.push_back(pointAt(i));
    }
    if (has_uniform_subdivision && has_loop() && points.size()>3)
    {
        points.push_back(points[1]);
        points.push_back(points[2]);
    }
    return points;
}

QVector<ControlPoint> BSpline::getPoints()
{
    if (subdivided_points.size() == 0)
    {
        recompute();
    }
    return subdivided_points;
}

void BSpline::computeControlPointNormals()
{
    inward_normals.clear();
    outward_normals.clear();
    inward_subdivided_normals.clear();
    outward_subdivided_normals.clear();
    for (int k=0; k<2; ++k)
    {
        start_has_zero_height[k] = !has_loop();
        end_has_zero_height[k] = !has_loop();
    }

    bool inverse = false;
    QVector<ControlPoint> cpts = getControlPoints();

    if (has_uniform_subdivision)
    {
        cpts.pop_back();
        cpts.pop_back();
        cpts.pop_back();
    }

    for (int i=0; i<m_splineGroup->junctionInfos.size();)
    {
        CurveJunctionInfo& junctionInfo = m_splineGroup->junctionInfos[i];
        if (junctionInfo.splineRef1 == ref)
        {
            m_splineGroup->junctionInfos.erase(m_splineGroup->junctionInfos.begin() + i);
            m_splineGroup->spline(junctionInfo.splineRef2).inward_subdivided_normals.clear();
        }
        else
            ++i;
    }
    QVector<CurveJunctionInfo> junctionInfos;

    if (cptRefs.size()>0 && has_loop())
    {
        QPointF inside_point = (QPointF)cpts.front() + 5*getNormal(cpts, 0);
        std::vector<cv::Point> contour;
        for (int i=0; i<cpts.size(); ++i)
        {
            contour.push_back(cv::Point(cpts[i].x(), cpts[i].y()));
        }
        if (cv::pointPolygonTest(contour, cv::Point2f(inside_point.x(), inside_point.y()), false) < 0)
        {
            inverse = true;
        }
    }

    for (int i=0; i<cpts.size(); ++i)
    {
        QPointF in_normal = getNormal(cpts, i);
        QPointF out_normal = -in_normal;

        if ((i == 0 || i==num_cpts()-1) &&  cpts[i].num_splines() > 1 && !(cpts[i].num_splines()==2 && has_loop()))
        {
            QVector<QLineF>  otherSplinesLines;
            QVector<int>  otherSplinesRefs;
            for (int k=0; k<cpts[i].num_splines(); ++k)
            {
                if (cpts[i].splineRefs[k] != ref || m_splineGroup->spline(cpts[i].splineRefs[k]).has_loop())
                {
                    BSpline& spline = m_splineGroup->spline(cpts[i].splineRefs[k]);
                    if (spline.num_cpts() <= 1) continue;

                    int j = -1;
                    for (int l=0; l< spline.num_cpts(); ++l)
                    {
                        if (spline.pointAt(l).ref == cpts[i].ref && !(spline.ref == ref && l==i))
                        {
                            j = l;
                            QLineF lineOther;
                            if (j == 0) lineOther = QLineF(spline.pointAt(1), spline.pointAt(0));
                            //else if (j==spline.num_cpts()-1)   lineOther=QLineF(spline.pointAt(cpts.size()-2), spline.pointAt(cpts.size()-1));
                            else    lineOther = QLineF(spline.pointAt(j-1), spline.pointAt(j));
                            lineOther = lineOther.unitVector();
                            otherSplinesLines.push_back(lineOther);
                            otherSplinesRefs.push_back(spline.ref);
                        }
                    }
                }
            }

            if (otherSplinesLines.size() > 0)
            {
                QLineF line;
                if (i == 0) line = QLineF(cpts[1], cpts.first());
                //else if (i==cpt.size()-1)   line=QLineF(cpts[cpts.size()-2], cpts.last());
                else    line = QLineF(cpts[i-1], cpts[i]);
                line = line.unitVector();

                int rightIndex = -1, leftIndex = -1;
                float maxAngle = FLT_MIN, minAngle = FLT_MAX;
                for (int l=0; l<otherSplinesLines.size(); ++l)
                {
                    float angle = line.angleTo(otherSplinesLines[l]);
                    if (angle > maxAngle)
                    {
                        maxAngle = angle;
                        rightIndex = l;
                    }
                    if (angle < minAngle)
                    {
                        minAngle = angle;
                        leftIndex = l;
                    }
                }

                if (i == 0)
                {
                    int tmp = leftIndex;
                    leftIndex = rightIndex;
                    rightIndex = tmp;
                }

                CurveJunctionInfo junctionInfo[2];

                QLineF line2 = otherSplinesLines[leftIndex];
                QPointF tangent = QPointF(line.dx(), line.dy()) - QPointF(line2.dx(), line2.dy());
                if (i == 0) tangent = -tangent;
                in_normal = QPointF(-tangent.y(), tangent.x());

                if (ref < otherSplinesRefs[leftIndex])
                {
                    junctionInfo[0].cptRef = pointAt(i).ref;
                    junctionInfo[0].splineRef1 = ref;
                    junctionInfo[0].splineRef2 = otherSplinesRefs[leftIndex];
                    junctionInfo[0].spline1Inward = true;
                    junctionInfo[0].spline1Normal = in_normal;
                    junctionInfos.push_back(junctionInfo[0]);                    
                } else if (ref > otherSplinesRefs[leftIndex])
                {
                    for (int l=0; l<m_splineGroup->junctionInfos.size(); ++l)
                    {
                        CurveJunctionInfo& junction  = m_splineGroup->junctionInfos[l];
                        if (junction.splineRef1==otherSplinesRefs[leftIndex] && junction.splineRef2 == ref)
                        {
                            if (fabs(in_normal.x()-junction.spline1Normal.x())<1e-8 &&
                                    fabs(in_normal.y()-junction.spline1Normal.y())<1e-8)
                            {
                                junction.spline2Inward = true;
                                BSpline& otherSpline = m_splineGroup->spline(otherSplinesRefs[leftIndex]);
                                bool curves_has_negative_directions = false;

                                float height1 = 0.0, height2 = 0.0;
                                if (i==0)   height1 = pointAt(1).attributes[0].height;
                                else    height1 = pointAt(num_cpts()-2).attributes[0].height;
                                if (otherSpline.cptRefs.first() == junction.cptRef)
                                    height2 = otherSpline.pointAt(1).attributes[!junction.spline1Inward].height;
                                else
                                    height2 = otherSpline.pointAt(otherSpline.num_cpts()-2).attributes[!junction.spline1Inward].height;
                                curves_has_negative_directions = ((height1*height2) < 0.0f);

                                junction.has_negative_directions = curves_has_negative_directions;
                                if (has_inward_surface &&
                                   ((otherSpline.has_outward_surface && !junction.spline1Inward) ||
                                    (otherSpline.has_inward_surface && junction.spline1Inward)))
                                {
                                    if (i==0)
                                        start_has_zero_height[0] = curves_has_negative_directions;
                                    else
                                        end_has_zero_height[0] = curves_has_negative_directions;

                                    if (otherSpline.cptRefs.first() == junction.cptRef)
                                        otherSpline.start_has_zero_height[!junction.spline1Inward] = curves_has_negative_directions;
                                    else
                                        otherSpline.end_has_zero_height[!junction.spline1Inward] = curves_has_negative_directions;
                                }
                            }
                        }
                    }
                }

                line2 = otherSplinesLines[rightIndex];
                tangent = -QPointF(line.dx(), line.dy()) + QPointF(line2.dx(), line2.dy());
                if (i == 0) tangent = -tangent;
                out_normal = QPointF(-tangent.y(), tangent.x());

                if (ref < otherSplinesRefs[rightIndex])
                {
                    junctionInfo[1].cptRef = pointAt(i).ref;
                    junctionInfo[1].splineRef1 = ref;
                    junctionInfo[1].splineRef2 = otherSplinesRefs[rightIndex];
                    junctionInfo[1].spline1Inward = false;
                    junctionInfo[1].spline1Normal = out_normal;
                    junctionInfos.push_back(junctionInfo[1]);
                }  else if (ref > otherSplinesRefs[rightIndex])
                {
                    for (int l=0; l<m_splineGroup->junctionInfos.size(); ++l)
                    {
                        CurveJunctionInfo& junction  = m_splineGroup->junctionInfos[l];
                        if (junction.splineRef1==otherSplinesRefs[rightIndex] && junction.splineRef2 == ref)
                        {
                            if (fabs(out_normal.x()-junction.spline1Normal.x())<1e-8 &&
                                    fabs(out_normal.y()-junction.spline1Normal.y())<1e-8)
                            {
                                junction.spline2Inward = false;
                                BSpline& otherSpline = m_splineGroup->spline(otherSplinesRefs[rightIndex]);
                                bool curves_has_negative_directions = false;

                                float height1 = 0.0, height2 = 0.0;
                                if (i==0)   height1 = pointAt(1).attributes[1].height;
                                else    height1 = pointAt(num_cpts()-2).attributes[1].height;
                                if (otherSpline.cptRefs.first() == junction.cptRef)
                                    height2 = otherSpline.pointAt(1).attributes[!junction.spline1Inward].height;
                                else
                                    height2 = otherSpline.pointAt(otherSpline.num_cpts()-2).attributes[!junction.spline1Inward].height;
                                curves_has_negative_directions = ((height1*height2) < 0.0f);

                                junction.has_negative_directions = curves_has_negative_directions;
                                if (has_outward_surface &&
                                   ((otherSpline.has_outward_surface && !junction.spline1Inward) ||
                                    (otherSpline.has_inward_surface && junction.spline1Inward)))
                                {
                                    if (i==0)
                                        start_has_zero_height[1] = curves_has_negative_directions;
                                    else
                                        end_has_zero_height[1] = curves_has_negative_directions;

                                    if (otherSpline.cptRefs.first() == junction.cptRef)
                                        otherSpline.start_has_zero_height[!junction.spline1Inward] = curves_has_negative_directions;
                                    else
                                        otherSpline.end_has_zero_height[!junction.spline1Inward] = curves_has_negative_directions;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!inverse)
        {
            inward_normals.push_back(in_normal);
            outward_normals.push_back(out_normal);
        }   else
        {
            inward_normals.push_back(out_normal);
            outward_normals.push_back(in_normal);
        }

    }


    for (int k=0; k<junctionInfos.size(); ++k)
        m_splineGroup->junctionInfos.push_back(junctionInfos[k]);

    QVector<ControlPoint> points = getPoints();
    for (int i=0; i<points.size(); ++i)
    {
        QPointF in_normal = getNormal(points, i);  if (inverse) in_normal = -in_normal;
        QPointF out_normal = -in_normal;

        if (!has_uniform_subdivision)   //Note: a curve has a uniform subdivision, it cannot have a junction point
        {
            if (i==0)
            {
                in_normal = inward_normals.first();
                out_normal = outward_normals.first();
            } else if (i==points.size()-1)
            {
                in_normal = inward_normals.last();
                out_normal = outward_normals.last();
            }
        }

        inward_subdivided_normals.push_back(unit(in_normal));
        outward_subdivided_normals.push_back(unit(out_normal));

    }
}
