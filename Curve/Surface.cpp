#include <fstream>
#include <QDebug>
#include "Surface.h"
#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"

#define EPSILON .00001f
#define angleT  90.0f


Surface::Surface():
    ref(-1), splineRef(-1), direction(INWARD_DIRECTION)
{
}

int Surface::addVertex(Point3d vertex)
{
    for (int k=0; k<vertices.size(); ++k)
    {
        if (fabs(vertices[k].x()-vertex.x()) < 1e-3 && fabs(vertices[k].y()-vertex.y()) < 1e-3 && fabs(vertices[k].z()-vertex.z()) < 1e-3)
            return k;
    }
    vertices.push_back(vertex);
    return vertices.size()-1;
}

int Surface::addVertex(QPointF point, float z)
{
    return addVertex(Point3d(point.x(), point.y(), z));
}

Point3d& Surface::pointAt(int u, int v)
{
    return vertices[controlMesh[u][v]];
}

void Surface::computeFaceIndices()
{
    faceIndices.clear();

    if (controlMesh.size() == 0 || controlMesh[0].size() == 0)
        return;

    //Compute faces indices
    //bool flip_face = true;
    for (int k=0; k<controlMesh.size()-1; ++k)
    {
        for (int l=0; l<controlMesh[k].size()-1; ++l)
        {
            QVector<int> indices;

            indices.push_back(controlMesh[k][l]);
            if (controlMesh[k][l+1] != indices.back())
                indices.push_back(controlMesh[k][l+1]);

            if (controlMesh[k+1][l+1] != indices.back())
                indices.push_back(controlMesh[k+1][l+1]);

            if (controlMesh[k+1][l] != indices.back() && controlMesh[k+1][l] != indices.front())
                indices.push_back(controlMesh[k+1][l]);

            /*if (flip_face)
                {
                    std::reverse(indices.begin(), indices.end());
                }*/

            if (indices.size() > 2)
                faceIndices.push_back(indices);
        }
    }

}

bool Surface::writeOFF(std::ostream &ofs)
{
    ofs << "OFF" << std::endl;
    if (vertices.size() == 0 || faceIndices.size() == 0)
    {
        ofs << 0 << " " << 0 << " " << 0 << std::endl;
        qWarning("Warning: This surface is empty!");
        return false;
    }

    ofs << vertices.size() << " " << faceIndices.size() << " " << sharpCorners.size() << std::endl;

    //Write vertices
    for (int i=0; i<vertices.size(); ++i)
    {
        ofs << vertices[i].x() << " " << vertices[i].y() << " " << vertices[i].z() << std::endl;
    }

    //Write faces
    for (int i=0; i<faceIndices.size(); ++i)
    {
        ofs << faceIndices[i].size() << " ";
        for (int m=0; m<faceIndices[i].size(); ++m)
            ofs << faceIndices[i][m] << " ";
        ofs << std::endl;
    }

    //Write sharp corners
    for (std::set<int>::iterator it = sharpCorners.begin(); it != sharpCorners.end(); ++it)
    {
        ofs << *it << std::endl;
    }

    return true;
}

void Surface::recompute(cv::Mat dt, cv::Mat luminance, bool clipHeight)
{

    vertices.clear();
    sharpCorners.clear();
    controlMesh.clear();

    bool inward = (direction == INWARD_DIRECTION);

    BSpline& bspline = m_splineGroup->spline(splineRef);
    QVector<ControlPoint> subdivided_points = bspline.getPoints();
    QVector<QPointF> normals = bspline.getNormals(inward);

    bool resubdivide = false;
    QVector<ControlPoint> original_points = bspline.getControlPoints();;
    for (int k=0; k<2; ++k)
    {        
        std::vector< std::pair<int, Attribute> > junctionPoints = bspline.junctionPoints[k];

        if (bspline.start_has_zero_height[k] || bspline.is_slope)
        {
            if (original_points.first().attributes[k].height*original_points[1].attributes[k].height < 0)
            {
                original_points.first().attributes[k].height = 0.0;
                resubdivide = true;
            }
        }
        if (bspline.end_has_zero_height[k] || bspline.is_slope)
        {
            if (original_points.last().attributes[k].height*original_points[original_points.size()-2].attributes[k].height < 0)
            {
                original_points.last().attributes[k].height = 0.0;
                resubdivide = true;
            }
        }

        if (junctionPoints.size() > 0)
        {
            resubdivide = true;
            for (int i=0; i<junctionPoints.size(); ++i)
            {
                NormalDirection tmp = original_points[junctionPoints[i].first].attributes[k].direction;
                original_points[junctionPoints[i].first].attributes[k] = junctionPoints[i].second;
                original_points[junctionPoints[i].first].attributes[k].direction = tmp;
            }
        }

    }

    if (resubdivide)
    {
        subdivided_points = subDivide(original_points, bspline.subv_levels, bspline.has_uniform_subdivision);
        if (bspline.has_uniform_subdivision && original_points.size() >= 4) {
            subdivided_points.pop_back();
            subdivided_points.pop_front();
        }
    }

    if (clipHeight && luminance.cols > 0)
    {
        QPointF point = (QPointF)subdivided_points[0] + std::min(subdivided_points[0].attribute(direction).extent,10.0f)*normals[0];
        int lightness = luminance.at<cv::Vec3b>(point.y(), point.x())[0];
        float l = lightness*100.0/255;
        //Clip heights according to luminance
        for (int i=0; i<subdivided_points.size(); ++i)
        {
            float height = subdivided_points[i].attribute(direction).height;

            if (height >= 0.0 && height > (100-l))
            {
                height = 100-l;
                subdivided_points[i].attribute(direction).height = height;
            } else if (height <=0 && height < -l)
            {
                height = -l;
                subdivided_points[i].attribute(direction).height = height;
            }
        }
    }
    QVector<QVector<int> > points = setSurfaceCP(subdivided_points, normals, dt,inward,bspline.has_loop(), bspline.start_has_zero_height[!inward], bspline.end_has_zero_height[!inward]);

    if(bspline.is_slope) {
        // find points on the other side
        normals = bspline.getNormals(inward);
        normals.pop_back();
        normals.pop_front();

        for (int i=0; i<subdivided_points.size(); ++i)
            subdivided_points[i].attributes[inward].height = subdivided_points[i].attributes[!inward].height;
        QVector<QVector<int> > points2 = setSurfaceCP(subdivided_points, bspline.getNormals(!inward), dt,!inward,false, true, true);

        for (int i=0; i<std::min(points.last().size(), points2.last().size()); ++i)
        {
            if (points.last()[i] != points2.last()[i])
            {
                if (points.last()[i] == points2.last()[i-1])
                    for (int k=0; k<points2.size(); ++k)
                    {
                        points2[k].insert(i, points2[k][i-1]);
                    }
                else if (points2.last()[i] == points.last()[i-1])
                    for (int k=0; k<points.size(); ++k)
                    {
                        points[k].insert(i, points[k][i-1]);
                    }
                ++i;
            }
        }

        /*Before merging, ensure that points2 and points do not have the same
          shape points*/
        for(int i=1;i<points2.size()-1;i++)
        {
            for (int k=0; k<points2[i].size(); ++k)
            {
                if (points2[i][k] == points[i][k])
                {
                    Point3d overlapping_vertex = vertices[points2[i][k]];
                    int new_vertex_id = addVertex(overlapping_vertex, 1000.0);
                    vertices[new_vertex_id].setZ(overlapping_vertex.z());
                    points2[i][k] = new_vertex_id;
                }
            }
        }

        // merge the two grids
        for(int i=0;i<points2.size()-1;i++)
            for(int j=points2[i].size()-2;j>=1;j--)
                points[i].push_back(points2[i][j]);
        for(int j=points2.last().size()-2;j>=1;j--)
            points.last().push_back(points.last()[j]);
        for(int i=0;i<points.size();i++)
            points[i].append(points[i].first());
    }


    for (int k=0; k<points.size(); ++k)
        controlMesh.append(points.at(k));

    // is this correct?
    /*for (int k=0; k<controlMesh.size(); ++k)
    {
        int start_pt_id = controlMesh[k][0];
        int end_pt_id = controlMesh[k][controlMesh[k].size()-1];
        sharpCorners.push_back(start_pt_id);
    */
        /*
         * This only set sharp corners on the control points with non-zero depth.
         * Remove "fabs(vertices[end_pt_id].z())<1.0" to set all appropriate corners (with depth or not)
         */
   /*     if (start_pt_id != end_pt_id && fabs(vertices[end_pt_id].z())>0.0)
        {
            sharpCorners.push_back(end_pt_id);
        }
    }*/

    computeFaceIndices();
    /*std::ofstream ofs("debug_surface.off");
    writeOFF(ofs);
    ofs.close();*/
}

QVector<QVector<int> > Surface::setSurfaceCP(QVector<ControlPoint> controlPoints, QVector<QPointF> normals, cv::Mat dt, bool inward, bool loop, bool start_has_zero_height, bool end_has_zero_height)
{
    // remove points from the tracing   (In the case of merging surfaces, we do this if the surfaces have an opposite)
    /*QVector<ControlPoint> endPoints;
    if(start_has_zero_height) {
        endPoints.append(controlPoints.first());
        controlPoints.pop_front();
        normals.pop_front();
    }
    if (end_has_zero_height)
    {
        endPoints.append(controlPoints.last());
        controlPoints.pop_back();
        normals.pop_back();
    }*/

    if (start_has_zero_height)
    {
        controlPoints.first().attributes[!inward].extent = 0.0;
        controlPoints.first().attributes[!inward].height = 0.0;
    }
    if (end_has_zero_height)
    {
        controlPoints.last().attributes[!inward].extent = 0.0;
        controlPoints.last().attributes[!inward].height = 0.0;
    }

    float cT = 90; // threshold for curvature (in degrees)
    NormalDirection direction = inward?INWARD_DIRECTION:OUTWARD_DIRECTION;

    QVector< QVector<int> > shape_controlpoints(controlPoints[0].attribute(direction).shapePointAtr.size()); // the set of control points that define the shape
    QVector<int> translated_cpts_ids;
    QVector<int> original_cpts_ids;     int extra_curvature_vertices_count = 0;

    for (int k=0; k<controlPoints.size(); ++k)
    {
        float height = controlPoints[k].attribute(direction).height;
        int vertexId = addVertex(controlPoints[k], height);
        original_cpts_ids.push_back( vertexId );
        if (k==0 && start_has_zero_height)
            sharpCorners.insert(vertexId);
        else if (k==controlPoints.size()-1 && end_has_zero_height)
            sharpCorners.insert(vertexId);
        else if (controlPoints[k].isSharp)
            sharpCorners.insert(vertexId);
    }

    // get limit points for the control points
    QVector<QPointF> lp = limitPoints(controlPoints);


    // loop through all control points for the given spline curve
    for (int k=0; k<controlPoints.size(); ++k)
    {
        QPointF normal = normals[k];
        float extent = controlPoints[k].attribute(direction).extent;
        float height = controlPoints[k].attribute(direction).height;
        QVector<QPointF> shapeAtrs = controlPoints[k].attribute(direction).shapePointAtr;
        QLineF normalL(lp.at(k),lp.at(k) + normal*extent);
        QPointF tmp = lp.at(k)+normal*2;
        QPoint current(qRound(tmp.x()),qRound(tmp.y()));
        QPointF new_cpt = (extent==0?lp.at(k):traceDT(dt,current,extent,normalL));

        // curvature check: add point if angle is above cT and check intersection with previous CP
        if(k>0) {
            Point3d prevCP1 = vertices[original_cpts_ids.at(k-1+extra_curvature_vertices_count)];
            Point3d  prevCP2 = vertices[translated_cpts_ids.last()];
            QLineF previousL = QLineF(prevCP1,prevCP2);
            Point3d thisCP = vertices[original_cpts_ids.at(k+extra_curvature_vertices_count)];
            QLineF thisL = QLineF(thisCP,new_cpt);
            float angle = std::min(previousL.angleTo(thisL),thisL.angleTo(previousL));
            if(angle>cT) {
                int vertexId = original_cpts_ids.at(k+extra_curvature_vertices_count);
                original_cpts_ids.insert(original_cpts_ids.begin()+k+extra_curvature_vertices_count, vertexId); ++extra_curvature_vertices_count;
                if (controlPoints[k].isSharp) sharpCorners.insert(vertexId);
                normal = new_cpt + 0.5*(prevCP2-new_cpt) - thisCP;
                float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
                if (norm > EPSILON)
                    normal /= norm;
                tmp = lp.at(k)+normal*3;
                current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
                tmp = (extent==0?lp.at(k):traceDTHighCurv(dt,current,extent,normal));

                translated_cpts_ids.push_back(addVertex(tmp));

                normal = tmp-(QPointF)controlPoints[k];

                for (int l=0; l<shapeAtrs.size(); ++l)
                {
                    int newId = addVertex((QPointF)controlPoints[k]+normal*shapeAtrs[l].x(),height*shapeAtrs[l].y());
                    shape_controlpoints[l].push_back(newId);
                }
            }
        }

        int vertexId = addVertex(Point3d(new_cpt.x(), new_cpt.y()));
        translated_cpts_ids.push_back(vertexId);
        //if (controlPoints[k].isSharp) sharpCorners.insert(vertexId);

        // add shape point
        normal = new_cpt-(QPointF)controlPoints[k];
        for (int l=0; l<shapeAtrs.size(); ++l)
        {
            vertexId = addVertex((QPointF)controlPoints[k]+normal*shapeAtrs[l].x(),height*shapeAtrs[l].y());
            shape_controlpoints[l].push_back(vertexId);
            //if (controlPoints[k].isSharp) sharpCorners.insert(vertexId);
        }
    }

    QVector<QVector<int> > points;
    points.append(translated_cpts_ids);
    for (int l = 0; l<shape_controlpoints.size(); ++l)
    {
        points.append(shape_controlpoints[l]);
    }
    points.append(original_cpts_ids);

    /*if(start_has_zero_height) {
        // add first point
        vertices[points.last().first()].setZ(0);
        QPointF second = vertices[points.last().first()];
        QPointF endP = (QPointF)endPoints.first();
        QPointF normal = endP-second;
        int id_cp = addVertex(endP);
        points.last().prepend(points.last().first());
        points.first().prepend(id_cp);
        QVector<QPointF> shapeAtrs = controlPoints.first().attribute(direction).shapePointAtr;
        for (int i=0; i<shapeAtrs.size(); ++i){
            int vertexId = addVertex(second+normal*shapeAtrs[i].x());
            vertices[points[i+1].first()].setZ(0);
            points[i+1].prepend(vertexId);
        }
    }

    if (end_has_zero_height)
    {
        // add last point
        vertices[points.last().last()].setZ(0);
        QPointF second = vertices[points.last().last()];
        QPointF endP = (QPointF)endPoints.last();
        QPointF normal = endP-second;
        int id_cp = addVertex(endP);
        points.last().append(points.last().last());
        points.first().append(id_cp);
        QVector<QPointF> shapeAtrs = controlPoints.last().attribute(direction).shapePointAtr;
        for (int i=0; i<shapeAtrs.size(); ++i){
            int vertexId = addVertex(second+normal*shapeAtrs[i].x());
            vertices[points[i+1].last()].setZ(0);
            points[i+1].append(vertexId);
        }
    }*/

    return points;
}

QPointF Surface::traceDT(cv::Mat dt,QPoint current,float width,QLineF normalL)
{
    float currentD = 0;
    QPointF new_cpt;

    while(true) {
        float oldD = currentD;
        QPoint m = localMax(dt,cv::Rect(current.x()-1,current.y()-1,current.x()+1,current.y()+1)
                            ,&currentD,normalL);
        // check lines
        if(fabs(oldD-currentD)<EPSILON || currentD >= width) {
            new_cpt.rx() = m.rx();
            new_cpt.ry() = m.ry();
            break;
        } else {
            current = m;
        }
    }

    return new_cpt;
}

QPoint Surface::localMax(cv::Mat I, cv::Rect N, float *oldD, QLineF normalL)
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
            if(fabs(d-m)<EPSILON)
                cand.append(QPoint(x,y));
            else if(d>m) {
                m=d;
                cand.clear();
                cand.append(QPoint(x,y));
            }
        }

    if(cand.count()==0)
        return QPoint(sx+1,sy+1);
    if(cand.count()==1) {
        *oldD = m;
        return cand.first();
    }

    // find smallest angle
    float sa = 360; // smallest angle
    QPoint winner;
    for (int i = 0;i<cand.count();i++) {
        QLineF currentL(normalL.p1(),cand.at(i));
        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        if(angle<sa) {
            sa = angle;
            winner = cand.at(i);
        }
    }

    *oldD = m;
    return winner;
}

QPoint Surface::traceDTHighCurv(cv::Mat dt,QPoint start,float width,QPointF normal)
{
    float currentD = 0;
    QPointF current = start;

    while(true) {
        float oldD = currentD;
        QPoint tmp = QPoint(qRound(current.x()),qRound(current.y()));
        currentD = dt.at<float>(tmp.y(),tmp.x());
        if(currentD < oldD || currentD >= width)
            return QPoint(qRound(current.x()),qRound(current.y()));
        else
            current = current+normal;
    }
}
