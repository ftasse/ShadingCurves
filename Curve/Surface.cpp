#include <fstream>
#include <QDebug>
#include "Surface.h"
#include "BSplineGroup.h"
#include "Utilities/SurfaceUtils.h"

#define EPSILON .00001f
#define angleT  35.0f


Surface::Surface():
    ref(-1), splineRef(-1), direction(INWARD_DIRECTION)
{
}

int Surface::addVertex(Point3d vertex)
{
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

QVector<QVector<int> > Surface::getFaceIndices()
{
    QVector<QVector<int> > faceIndices;

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

            faceIndices.push_back(indices);
        }
        //flip_face = !flip_face;
    }

    return faceIndices;
}

bool Surface::writeOFF(std::ostream &ofs)
{
    ofs << "OFF" << std::endl;
    if (controlMesh.size() == 0)
    {
        ofs << 0 << " " << 0 << " " << 0 << std::endl;
        qWarning("Warning: This surface is empty!");
        return false;
    }

    QVector<QVector<int> > faceIndices = getFaceIndices();
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
    for (int i=0; i<sharpCorners.size(); ++i)
    {
        ofs << sharpCorners[i] << std::endl;
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

    // HENRIK: what's the point of this? Can't we just pass a QList<QPointF> to setSurface?
    QVector<ControlPoint> subdivided_points = bspline.getPoints();

    QVector<QVector<int> > points = setSurfaceCP(subdivided_points,dt,inward);

    if(bspline.is_slope) {
        // find points on the other side
        QVector<QVector<int> > points2 = setSurfaceCP(subdivided_points,dt,!inward);

        // set end points to zero
        vertices[points[1][0]].setZ(0);
        vertices[points2[1][0]].setZ(0);
        vertices[points[1][points[1].size()-1]].setZ(0);
        vertices[points2[1][points2[1].size()-1]].setZ(0);

        // add additional point at end points (Note that points.first() are the translated points and points.last() are the original points)
        QVector<QPointF> shapeAtrs = subdivided_points.first().attributes[0].shapePointAtr;
        float extent = subdivided_points.first().attributes[0].extent;
        QPointF cp = vertices[points.last().first()]; // end control point
        QPointF cp1 = vertices[points.first().first()]; // first translated point
        QPointF cp2 = vertices[points2.first().first()]; // second translated point (on the other side)
        QPointF tangent = cp1-cp2;
        QPointF normal = QPointF(-tangent.y(),tangent.x());
        float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        QLineF normalL(cp,cp + normal*extent);
        QPointF tmp = cp+normal*2;
        QPoint current(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,extent);
        normal = tmp-cp;

        // add first point?
        int id_cp = addVertex(cp);
        points.last().prepend(id_cp);
        for (int i=0; i<shapeAtrs.size(); ++i) {
            id_cp = addVertex(cp+normal*shapeAtrs[i].x());
            points[i+1].prepend(id_cp);
        }
        id_cp = addVertex(tmp);
        points.first().prepend(id_cp);

        // close the loop
        id_cp = addVertex(cp);
        points2.last().prepend(id_cp);
        for (int i=0; i<shapeAtrs.size(); ++i) {
            id_cp = addVertex(cp+normal*shapeAtrs[i].x());
            points2[i+1].prepend(id_cp);
        }
        id_cp = addVertex(tmp);
        points2.first().prepend(id_cp);

        // do something similar on the other side
        shapeAtrs = subdivided_points.last().attributes[0].shapePointAtr;
        extent = subdivided_points.last().attributes[0].extent;
        cp = vertices[points.last().last()];
        cp1 = vertices[points.first().last()];
        cp2 = vertices[points2.first().last()];
        tangent = cp2-cp1;
        normal = QPointF(-tangent.y(),tangent.x());
        norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        normalL = QLineF(cp,cp + normal*extent);
        tmp = cp+normal*2;
        current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,extent);
        normal = tmp-cp;

        // add middle point
        id_cp = addVertex(cp);
        points.last().append(id_cp);
        for (int i=0; i<shapeAtrs.size(); ++i) {
            id_cp = addVertex(cp+normal*shapeAtrs[i].x());
            points[i+1].append(id_cp); qDebug() << id_cp;
        }
        id_cp = addVertex(tmp);
        points.first().append(id_cp);

        // merge the two grids
        for(int i=0;i<points2.size();i++)
            for(int j=points2[i].size()-1;j>=0;j--)
                points[i].push_back(points2[i][j]);

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

    std::ofstream ofs("debug_surface.off");
    writeOFF(ofs);
    ofs.close();
}

QVector<QVector<int> > Surface::setSurfaceCP(QVector<ControlPoint> controlPoints, cv::Mat dt, bool inward)
{
    float cT = 90; // threshold for curvature (in degrees)
    NormalDirection direction = inward?INWARD_DIRECTION:OUTWARD_DIRECTION;

    QVector< QVector<int> > shape_controlpoints(controlPoints[0].attribute(direction).shapePointAtr.size()); // the set of control points that define the shape
    QVector<int> translated_cpts_ids;
    QVector<int> original_cpts_ids;

    for (int k=0; k<controlPoints.size(); ++k)
    {
        /*if (k == bspline_vertexIds.size()-1 && controlPoints.has_loop()) //if closed curve. HENRIK: why should this happen here, and not before?
        {
            original_cpts_ids.push_back(original_cpts_ids[0]);
        } else
        {
            Point3d new_cpt = vertices[bspline_vertexIds[k]];
            new_cpt.setZ(z);
            int vertexId = addVertex(new_cpt);
            original_cpts_ids.push_back(vertexId);
        }*/

        original_cpts_ids.push_back( addVertex(controlPoints[k]) );
    }

    // get limit points for the control points
    QVector<QPointF> lp = limitPoints(controlPoints);

    // loop through all control points for the given spline curve
    for (int k=0; k<controlPoints.size(); ++k)
    {
        /*if (k == bspline_vertexIds.size()-1 && controlPoints.has_loop()) //if closed curve
        {
            if (!controlPoints.has_uniform_subdivision)
                 translated_cpts_ids.push_back( translated_cpts_ids[0]);
            else
                 translated_cpts_ids.push_back( translated_cpts_ids[0]); //FLORA, found out what this should be
        } else*/
        {
            QPointF normal = getNormal(controlPoints, k);
            float extent = controlPoints[k].attribute(direction).extent;
            QVector<QPointF> shapeAtrs = controlPoints[k].attribute(direction).shapePointAtr;
            if(!inward)
                normal = -normal;
            QLineF normalL(lp.at(k),lp.at(k) + normal*extent);
            QPointF tmp = lp.at(k)+normal*2;
            QPoint current(qRound(tmp.x()),qRound(tmp.y()));
            QPointF new_cpt = traceDT(dt,lp.at(k),current,normalL,extent);

            // curvature check: add point if angle is above cT
            if(k>0) {
                Point3d prevCP1 = vertices[original_cpts_ids.at(k-1)];
                Point3d  prevCP2 = vertices[translated_cpts_ids.last()];
                QLineF previousL = QLineF(prevCP1,prevCP2);
                Point3d thisCP = vertices[original_cpts_ids.at(k)];
                QLineF thisL = QLineF(thisCP,new_cpt);
                float angle = std::min(previousL.angleTo(thisL),thisL.angleTo(previousL));
                if(angle>cT) {
                    original_cpts_ids.insert(original_cpts_ids.begin()+k, addVertex(prevCP1));
                    QPointF tangent = thisCP-prevCP1;
                    normal = QPointF(-tangent.y(),tangent.x());
                    if(!inward) normal = -normal;
                    float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
                    if (norm > EPSILON)
                        normal /= norm;
                    normalL = QLineF(lp.at(k),lp.at(k) + normal*extent);
                    tmp = lp.at(k)+normal*5;
                    current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
                    tmp = traceDT(dt,lp.at(k),current,normalL,extent);

                    translated_cpts_ids.push_back(addVertex(tmp));

                    normal = tmp-lp.at(k);

                    for (int l=0; l<shapeAtrs.size(); ++l)
                    {
                        int newId = addVertex(lp.at(k)+normal*shapeAtrs[l].x(),controlPoints[k].z()*shapeAtrs[l].y());
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
                vertexId = addVertex(lp.at(k)+normal*shapeAtrs[l].x(),controlPoints[k].z()*shapeAtrs[l].y());
                shape_controlpoints[l].push_back(vertexId);
            }
        }
    }
    QVector<QVector<int> > points;
    points.append(translated_cpts_ids);
    for (int l = 0; l<shape_controlpoints.size(); ++l)
    {
        points.append(shape_controlpoints[l]);
    }
    points.append(original_cpts_ids);
    return points;
}

QPointF Surface::traceDT(cv::Mat dt,QPointF limit,QPoint current,QLineF normalL,float width)
{
    // thresholds
    float Td = .75f; // for distance
    float Ta = 1.0f; // for angle


    float currentD = 0;
    QPointF new_cpt;
    QList<QPoint> visited;

    while(true) {
        float oldD = currentD;
        QPoint m = localMax(dt,cv::Rect(current.x()-1,current.y()-1,current.x()+1,current.y()+1)
                            ,&currentD,normalL,visited,Td,Ta);
        // check lines
        QLineF currentL(limit,m);
        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        if(fabs(oldD-currentD)<EPSILON || currentD >= width || angle > angleT) {
            new_cpt.rx() = m.rx();
            new_cpt.ry() = m.ry();
            break;
        } else {
            visited.append(current);
            current = m;
        }
    }

    return new_cpt;
}

// HENRIK: find max value in I, in neighbourhood N
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
    QList<float> angles;
    for (int i = 0;i<cand.count();i++) {
        QLineF currentL(normalL.p1(),cand.at(i));
        float angle = std::min(currentL.angleTo(normalL),normalL.angleTo(currentL));
        angles.append(angle);
        if(angle<sa)
            sa = angle;
    }

    // pick max candidate
    QPoint winner;
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
