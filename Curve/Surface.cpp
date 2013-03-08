#include <fstream>
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
    bool flip_face = true;
    for (int k=1; k<controlMesh.size(); ++k)
    {
        for (int l=0; l<controlMesh[0].size()-1; ++l)
        {
            QVector<int> indices;

            indices.push_back(controlMesh[0][l]);
            if (controlMesh[0][l+1] != indices.back())
                indices.push_back(controlMesh[0][l+1]);

            if (controlMesh[k][l+1] != indices.back())
                indices.push_back(controlMesh[k][l+1]);

            if (controlMesh[k][l] != indices.back() && controlMesh[k][l] != indices.front())
                indices.push_back(controlMesh[k][l]);

            if (flip_face)
            {
                std::reverse(indices.begin(), indices.end());
            }

            faceIndices.push_back(indices);
        }
        flip_face = !flip_face;
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
}

void Surface::recompute(cv::Mat dt)
{

    vertices.clear();
    sharpCorners.clear();
    controlMesh.clear();
    bspline_vertexIds.clear();


    bool inward = (direction == INWARD_DIRECTION);

    /* FLORA: z and width should not be hardcoded, but obtained from the control points */
    int z;
    if (inward)
    {
        z = -50;
    }
    else
    {
        z = 50;
    }
    float width = 50.0;


    BSpline& bspline = m_splineGroup->spline(splineRef);
    // bspline.fix_orientation();
/*
    for (int i=0; i<bspline.num_cpts(); ++i)
    {
        /*
         *  From Flora:
         *  Note that you can get attributes for inward direction from cpt.attributes[0] and for outward direction from cpt.attributes[1]
         */
        //ControlPoint& cpt = bspline.pointAt(i);
//    }

    QVector<QPointF> subdivided_points = bspline.getPoints();
    for (int i=0; i<subdivided_points .size(); ++i)
    {
        //FLORA: May be the z value, instead of 0.0f, should be the height in cpt attribute ? maybe via some interpolation in the subdivision
        Point3d point(subdivided_points[i].x(), subdivided_points[i].y(), 0.0f);
        bspline_vertexIds.push_back(addVertex(Point3d(point.x(), point.y(), point.z())));
    }

    QVector<QVector<int> > points = setSurfaceCP(bspline,dt,z,width,inward,false);

    if(bspline.is_slope) {
        // find points on the other side
        QVector<QVector<int> > points2 = setSurfaceCP(bspline,dt,z,width,!inward,true);

        // set end points to zero
        vertices[points[1][0]].setZ(0);
        vertices[points2[1][0]].setZ(0);
        vertices[points[1][points[1].size()-1]].setZ(0);
        vertices[points2[1][points2[1].size()-1]].setZ(0);

        // add additional point at end points
        QPointF cp = vertices[points[0][0]]; // end control point
        QPointF cp1 = vertices[points[2][0]]; // first translated point
        QPointF cp2 = vertices[points2[2][0]]; // second translated point (on the other side)
        QPointF tangent = cp1-cp2;
        QPointF normal = QPointF(-tangent.y(),tangent.x());
        float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        QLineF normalL(cp,cp + normal*width);
        QPointF tmp = cp+normal*2;
        QPoint current(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,width);

        // add first point?
        int id_cp = addVertex(cp);
        points[0].prepend(id_cp);
        id_cp = addVertex(vertices[points[1][0]]);
        points[1].prepend(id_cp);
        id_cp = addVertex(tmp);
        points[2].prepend(id_cp);

        // close the loop
        id_cp = addVertex(cp);
        points2[0].prepend(id_cp);
        id_cp = addVertex(vertices[points[1][0]]);
        points2[1].prepend(id_cp);
        id_cp = addVertex(tmp);
        points2[2].prepend(id_cp);

        // do something similar on the other side
        cp = vertices[points[0][points[0].size()-1]];
        cp1 = vertices[points[2][points[2].size()-1]];
        cp2 = vertices[points2[2][points[2].size()-1]];
        tangent = cp2-cp1;
        normal = QPointF(-tangent.y(),tangent.x());
        norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        normalL = QLineF(cp,cp + normal*width);
        tmp = cp+normal*2;
        current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,width);

        // add middle point
        id_cp = addVertex(cp);
        points[0].append(id_cp);
        id_cp = addVertex(vertices[points[1][points[1].size()-1]]);
        points[1].append(id_cp);
        id_cp = addVertex(tmp);
        points[2].append(id_cp);

        // merge the two grids
        for(int i=0;i<points2.size();i++)
            for(int j=points2[i].size()-1;j>=0;j--)
                points[i].push_back(points2[i][j]);
    }

    controlMesh.append(points.at(0));
    controlMesh.append(points.at(1));
    controlMesh.append(points.at(2));

    for (int k=0; k<controlMesh.size(); ++k)
    {
        int start_pt_id = controlMesh[k][0];
        int end_pt_id = controlMesh[k][controlMesh[k].size()-1];
        sharpCorners.push_back(start_pt_id);

        /*
         * This only set sharp corners on the control points with non-zero depth.
         * Remove "fabs(vertices[end_pt_id].z())<1.0" to set all appropriate corners (with depth or not)
         */
        if (start_pt_id != end_pt_id && fabs(vertices[end_pt_id].z())<1.0)
        {
            sharpCorners.push_back(end_pt_id);
        }
    }

    /*std::ofstream ofs("debug_surface.off");
    writeOFF(ofs);
    ofs.close();*/
}

QVector<QVector<int> > Surface::setSurfaceCP(BSpline& bspline,cv::Mat dt,float z,float width,bool inward, bool newP)
{
    float cT = 90; // threshold for curvature (in degrees)

    QVector<int> original;
    if(newP) {
        for(int i=0;i<bspline_vertexIds.size();i++)
        {
            int vertexId = bspline_vertexIds[i];
            original.append(addVertex(vertices[vertexId]));
        }
    } else
        original = bspline_vertexIds;
    QVector<int> translated_cpts_ids;
    QVector<int> perpendicular_cpts_ids;

    for (int k=0; k<bspline_vertexIds.size(); ++k)
    {
        if (k == bspline_vertexIds.size()-1 && bspline.has_loop()) //if closed curve
        {
            if (!bspline.has_uniform_subdivision)
                perpendicular_cpts_ids.push_back(perpendicular_cpts_ids[0]);
            else
                perpendicular_cpts_ids.push_back(perpendicular_cpts_ids[0]); //FLORA, found out what this should be
        } else
        {
            Point3d new_cpt = vertices[bspline_vertexIds[k]];
            new_cpt.setZ(z);
            int vertexId = addVertex(new_cpt);
            perpendicular_cpts_ids.push_back(vertexId);
        }
    }

    // get limit points for the control points
    QVector<QPointF> lp = limitPoints(bspline.getPoints());

    // loop through all control points for the given spline curve
    for (int k=0; k<bspline_vertexIds.size(); ++k)
    {
        if (k == bspline_vertexIds.size()-1 && bspline.has_loop()) //if closed curve
        {
            if (!bspline.has_uniform_subdivision)
                 translated_cpts_ids.push_back( translated_cpts_ids[0]);
            else
                 translated_cpts_ids.push_back( translated_cpts_ids[0]); //FLORA, found out what this should be
        } else
        {
            // HENRIK: move in the distance transform image
            QPointF normal = bspline.inward_normal(k, true);
            if(!inward)
                normal = -normal;
            QLineF normalL(lp.at(k),lp.at(k) + normal*width);
            QPointF tmp = lp.at(k)+normal*2;
            QPoint current(qRound(tmp.x()),qRound(tmp.y()));
            QPointF new_cpt = traceDT(dt,lp.at(k),current,normalL,width);

            // curvature check
            if(k>0) {
                QPointF prevCP1 = QPointF(vertices[original.at(k-1)].x(),vertices[original.at(k-1)].y());
                QPointF prevCP2 = QPointF(vertices[translated_cpts_ids.last()].x(),vertices[translated_cpts_ids.last()].y());
                QLineF previousL = QLineF(prevCP1,prevCP2);
                QPointF thisCP = QPointF(vertices[original.at(k)].x(),vertices[original.at(k)].y());
                QLineF thisL = QLineF(thisCP,new_cpt);
                float angle = std::min(previousL.angleTo(thisL),thisL.angleTo(previousL));
                if(angle>cT) {
                    original.insert(original.begin()+k, addVertex(prevCP1));
                    perpendicular_cpts_ids.insert(perpendicular_cpts_ids.begin()+k, addVertex(prevCP1, z));
                    QPointF tangent = thisCP-prevCP1;
                    normal = QPointF(-tangent.y(),tangent.x());
                    if(!inward) normal = -normal;
                    float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
                    if (norm > EPSILON)
                        normal /= norm;
                    normalL = QLineF(lp.at(k),lp.at(k) + normal*width);
                    tmp = lp.at(k)+normal*5;
                    current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
                    tmp = traceDT(dt,lp.at(k),current,normalL,width);

                    translated_cpts_ids.push_back(addVertex(tmp));
                }
            }

            int vertexId = addVertex(Point3d(new_cpt.x(), new_cpt.y()));
            translated_cpts_ids.push_back(vertexId);
        }
    }

    QVector<QVector<int> > points;
    points.append(original);
    points.append(perpendicular_cpts_ids);
    points.append(translated_cpts_ids);
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
