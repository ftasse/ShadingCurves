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

    if (columnBreaks.size() == 0 && hasBreak.size() == controlMesh[0].size())
    {
        for (int i=0; i<hasBreak.size(); ++i)
        {
            if (hasBreak[i])
                columnBreaks.append(i);
        }
    }

    QVector<int> breaks = columnBreaks;
    breaks.append(controlMesh[0].size()-1);

    //Compute faces indices
    //bool flip_face = true;
    for (int k=0; k<controlMesh.size()-1; ++k)
    {
        int l = 0;
        for (int i=0; i<breaks.size(); ++i)
        {
            int next_break = breaks[i];
            while (l < next_break)
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

                ++l;
            }
            ++l;

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
    for (QSet<int>::iterator it = sharpCorners.begin(); it != sharpCorners.end(); ++it)
    {
        ofs << *it << std::endl;
    }

    return true;
}

void Surface::recompute(cv::Mat dt)
{

    vertices.clear();
    sharpCorners.clear();
    controlMesh.clear();

    bool inward = (direction == INWARD_DIRECTION);

    BSpline& bspline = m_splineGroup->spline(splineRef);

    QVector<ControlPoint> subdivided_points = bspline.getPoints();
    QVector<QPointF> normals = bspline.getNormals(inward);

    QVector<QVector<int> > points = setSurfaceCP(subdivided_points, normals, dt,inward,bspline.has_loop(), bspline.start_has_zero_height[!inward], bspline.end_has_zero_height[!inward]);

    if(bspline.is_slope) {
        // find points on the other side
        normals = bspline.getNormals(inward);
        normals.pop_back();
        normals.pop_front();
        QVector<QVector<int> > points2 = setSurfaceCP(subdivided_points, bspline.getNormals(!inward), dt,!inward,false, true, true);

        if (points.last().size() > points2.last().size())
        {
            for (int i=0; i<points2.last().size(); ++i)
            {
                Point3d p1 = vertices[points.last()[i]];
                Point3d p2 = vertices[points2.last()[i]];
                if (fabs(p1.x()-p2.x()) > 1e-3 || fabs(p1.y()-p2.y()) > 1e-3 || fabs(p1.z()-p2.z()) > 1e-3)
                {
                    for (int k=0; k<points2.size(); ++k)
                    {
                        points2[k].insert(i, points2[k][i]);
                    }
                    ++i;
                }
            }
        } else if (points.last().size() < points2.last().size())
        {
            for (int i=0; i<points.last().size(); ++i)
            {
                Point3d p1 = vertices[points.last()[i]];
                Point3d p2 = vertices[points2.last()[i]];
                if (fabs(p1.x()-p2.x()) > 1e-3 || fabs(p1.y()-p2.y()) > 1e-3 || fabs(p1.z()-p2.z()) > 1e-3)
                {
                    for (int k=0; k<points.size(); ++k)
                    {
                        points[k].insert(i, points[k][i]);
                    }
                    ++i;
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
    std::ofstream ofs("debug_surface.off");
    writeOFF(ofs);
    ofs.close();
}

QVector<QVector<int> > Surface::setSurfaceCP(QVector<ControlPoint> controlPoints, QVector<QPointF> normals, cv::Mat dt, bool inward, bool loop, bool start_has_zero_height, bool end_has_zero_height)
{
    // remove points from the tracing   (In the case of merging surfaces, we do this if the surfaces have an opposite)
    QVector<ControlPoint> endPoints;
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
    }

    float cT = 90; // threshold for curvature (in degrees)
    NormalDirection direction = inward?INWARD_DIRECTION:OUTWARD_DIRECTION;

    QVector< QVector<int> > shape_controlpoints(controlPoints[0].attribute(direction).shapePointAtr.size()); // the set of control points that define the shape
    QVector<int> translated_cpts_ids;
    QVector<int> original_cpts_ids;     int extra_curvature_vertices_count = 0;

    for (int k=0; k<controlPoints.size(); ++k)
    {
        float height = controlPoints[k].attribute(direction).height;
        if (k==0 && start_has_zero_height)  height = 0.0f;
        else if (k==controlPoints.size()-1 && end_has_zero_height)  height = 0.0f;
        original_cpts_ids.push_back( addVertex(controlPoints[k], height) );
        if (controlPoints[k].isSharp)
                sharpCorners.insert(original_cpts_ids.last());
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
        QPointF tmp = lp.at(k)+normal*4;
        QPoint current(qRound(tmp.x()),qRound(tmp.y()));
        QPointF new_cpt = traceDT(dt,lp.at(k),current,normalL,extent,false);

        // curvature check: add point if angle is above cT and check intersection with previous CP
        if(k>0) {
            Point3d prevCP1 = vertices[original_cpts_ids.at(k-1+extra_curvature_vertices_count)];
            Point3d  prevCP2 = vertices[translated_cpts_ids.last()];
            QLineF previousL = QLineF(prevCP1,prevCP2);
            Point3d thisCP = vertices[original_cpts_ids.at(k+extra_curvature_vertices_count)];
            QLineF thisL = QLineF(thisCP,new_cpt);
            float angle = std::min(previousL.angleTo(thisL),thisL.angleTo(previousL));
            if(angle>cT) {
                original_cpts_ids.insert(original_cpts_ids.begin()+k+extra_curvature_vertices_count, addVertex(prevCP1)); ++extra_curvature_vertices_count;
                QPointF tangent = thisCP-prevCP1;
                normal = QPointF(-tangent.y(),tangent.x());
                if(!inward) normal = -normal;
                float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
                if (norm > EPSILON)
                    normal /= norm;
                normalL = QLineF(lp.at(k),lp.at(k) + normal*extent);
                tmp = lp.at(k)+normal*5;
                current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
                tmp = traceDT(dt,lp.at(k),current,normalL,extent,true);

                translated_cpts_ids.push_back(addVertex(tmp));

                normal = tmp-lp.at(k);

                for (int l=0; l<shapeAtrs.size(); ++l)
                {
                    int newId = addVertex(lp.at(k)+normal*shapeAtrs[l].x(),height*shapeAtrs[l].y());
                    shape_controlpoints[l].push_back(newId);
                }
            }
        }

        int vertexId = addVertex(Point3d(new_cpt.x(), new_cpt.y()));
        translated_cpts_ids.push_back(vertexId);

        // add shape point
        normal = new_cpt-lp.at(k);
        for (int l=0; l<shapeAtrs.size(); ++l)
        {
            vertexId = addVertex(lp.at(k)+normal*shapeAtrs[l].x(),height*shapeAtrs[l].y());
            shape_controlpoints[l].push_back(vertexId);
        }
    }

    QVector<QVector<int> > points;
    points.append(translated_cpts_ids);
    for (int l = 0; l<shape_controlpoints.size(); ++l)
    {
        points.append(shape_controlpoints[l]);
    }
    points.append(original_cpts_ids);

    if(start_has_zero_height) {
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
    }

    return points;
}

QPointF Surface::traceDT(cv::Mat dt,QPointF limit,QPoint current,QLineF normalL,float width,bool normal)
{
    // thresholds
    float Td = .000001f;
    float Ta = .000001f;
    if(normal) { // trace along the normal direction by setting the thresholds
 //       a = setThresholds(normalL);
 //       Td = a*0.75f+(1-a)*0.6;
 //       Ta = a*0.1f+(1-a)*0.75f;
        Td = .8f;
        Ta = 1.0f;
    }

    float currentD = 0;
    QPointF new_cpt;
    QList<QPoint> visited;
//    float counter = 0;

    while(true) {
        float oldD = currentD;
        QPoint m = localMax(dt,cv::Rect(current.x()-1,current.y()-1,current.x()+1,current.y()+1)
                            ,&currentD,normalL,visited,Td,Ta);

        // check lines
//        QLineF currentL(limit,m);
//        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        if(fabs(oldD-currentD)<EPSILON || currentD >= width) { // || angle > angleT || counter>width) {
            new_cpt.rx() = m.rx();
            new_cpt.ry() = m.ry();
            break;
        } else {
            visited.append(current);
            current = m;
 //           counter++;
        }
    }

    return new_cpt;
}

QPoint Surface::localMax(cv::Mat I, cv::Rect N, float *oldD, QLineF normalL, QList<QPoint> visited, float Td, float Ta)
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
            if(fabs(d-m)<Td && !visCheck)
                cand.append(QPoint(x,y));
            else if(d>m) {
                m=d;
                cand.clear();
                cand.append(QPoint(x,y));
            }
            assert(!(d-m>EPSILON && visCheck));
        }

    if(cand.count()==0)
        return QPoint(sx+1,sy+1);

    // find smallest angle
    float sa = 360; // smallest angle
    int index;
    QList<float> angles;
    for (int i = 0;i<cand.count();i++) {
        QLineF currentL(normalL.p1(),cand.at(i));
        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        angles.append(angle);
        if(angle<sa) {
            sa = angle;
            index = i;
        }
    }

    // pick max candidate
    QPoint winner = cand[index];
    m = -1;
    for (int i = 0;i<cand.count();i++) {
        if(angles.at(i)<sa+Ta) {
            QPoint tmp = cand.at(i);
            float d = I.at<float>(tmp.y(),tmp.x());
            if(d>m) {
                winner = tmp;
                m = d;
            }
        }
    }

    *oldD = m;
    return winner;
}

// TODO: rename
float Surface::setThresholds(QLineF normal)
{
    QLineF X(.0f,.0f,1.0f,.0f);
    float angle = std::min(normal.angleTo(X),X.angleTo(normal));
    float d;
    if(angle<45.0f)
        d = 1-angle/45;
    else if(angle>=45.0f&&angle<90.0f)
        d = (angle-45.0f)/45.0f;
    else if(angle>=90.0f&&angle<135.0f)
        d = 1-(angle-90.0f)/45.0f;
    else
        d = (angle-135.0f)/45.0f;

    return d;
}
