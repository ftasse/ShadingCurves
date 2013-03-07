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

    /*vertices.clear();
    sharpCorners.clear();
    controlMesh.clear();

    float width = 50.0;
    bool inward = true;
    int z;
    bool slope = false;

    if (inward)
    {
        z = -50;
    }
    else
    {
        z = 50;
    }
    BSpline& bspline = m_splineGroup->spline(splineRef);

    //    bspline.fix_orientation();

    QVector<QVector<int> > points = setSurfaceCP(bspline,dt,z,width,inward,false);

    if(slope) {
        // find points on the other side
        QVector<QVector<int> > points2 = setSurfaceCP(bspline,dt,z,width,!inward,true);

        // set end points to zero
        controlPoint(points[1][0]).setZ(0);
        controlPoint(points2[1][0]).setZ(0);
        controlPoint(points[1][points[1].size()-1]).setZ(0);
        controlPoint(points2[1][points2[1].size()-1]).setZ(0);

        // add additional point at end points
        QPointF cp = controlPoint(points[0][0]); // end control point
        QPointF cp1 = controlPoint(points[2][0]); // first translated point
        QPointF cp2 = controlPoint(points2[2][0]); // second translated point (on the other side)
        QPointF tangent = cp1-cp2;
        QPointF normal = QPointF(-tangent.y(),tangent.x());
        float norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        QLineF normalL(cp,cp + normal*width);
        QPointF tmp = cp+normal*2;
        QPoint current(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,width);

        // is this dumb?
        float id_cp = addControlPoint(cp);
        points[0].prepend(id_cp);
        id_cp = addControlPoint(controlPoint(points[1][0]));
        points[1].prepend(id_cp);
        id_cp = addControlPoint(tmp);
        points[2].prepend(id_cp);

        // close the loop
        id_cp = addControlPoint(cp);hange_bspline_parameters()
        points2[0].prepend(id_cp);
        id_cp = addControlPoint(controlPoint(points[1][0]));
        points2[1].prepend(id_cp);
        id_cp = addControlPoint(tmp);
        points2[2].prepend(id_cp);

        // do something similar on the other side
        cp = controlPoint(points[0][points[0].size()-1]); // end control point
        cp1 = controlPoint(points[2][points[2].size()-1]); // first translated point
        cp2 = controlPoint(points2[2][points[2].size()-1]); // second translated point (on the other side)
        tangent = cp2-cp1;
        normal = QPointF(-tangent.y(),tangent.x());
        norm = sqrt(normal.x()*normal.x() + normal.y()*normal.y());
        if (norm > EPSILON)
            normal /= norm;
        normalL = QLineF(cp,cp + normal*width);
        tmp = cp+normal*2;
        current = QPoint(qRound(tmp.x()),qRound(tmp.y()));
        tmp = traceDT(dt,cp,current,normalL,width);

        // is this dumb?
        id_cp = addControlPoint(cp);
        points[0].append(id_cp);
        id_cp = addControlPoint(controlPoint(points[1][points[1].size()-1]));
        points[1].append(id_cp);
        id_cp = addControlPoint(tmp);
        points[2].append(id_cp);

        for(int i=0;i<points2.size();i++)
            for(int j=points2[i].size()-1;j>=0;j--)
                points[i].push_back(points2[i][j]);
    }

    surf.controlPoints().append(points.at(0));
    surf.controlPoints().append(points.at(1));
    surf.controlPoints().append(points.at(2));

    std::ofstream ofs("debug_surface.off");
    writeOFF(ofs);
    ofs.close();*/
}

QVector<QVector<int> > Surface::setSurfaceCP(BSpline& bspline,cv::Mat dt,float z,float width,bool inward, bool newP)
{
    /*float cT = 90; // threshold for curvature (in degrees)

    QVector<int> original;
    if(newP) {
        for(int i=0;i<bspline.connected_cpts.size();i++)
            original.append(addControlPoint(controlPoint(bspline.connected_cpts[i])));
    } else
        original = bspline.connected_cpts;
    QVector<int> translated_cpts_ids;
    QVector<int> perpendicular_cpts_ids;

    for (int k=0; k<bspline.count(); ++k)
    {
        if (k == bspline.count()-1 && bspline.has_loop()) //if closed curve
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
        if (k == bspline.count()-1 && bspline.has_loop()) //if closed curve
        {
            translated_cpts_ids.push_back(translated_cpts_ids[0]);
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
                QPointF prevCP1 = QPointF(controlPoint(original.at(k-1)).x(),controlPoint(original.at(k-1)).y());
                QPointF prevCP2 = QPointF(controlPoint(translated_cpts_ids.last()).x(),controlPoint(translated_cpts_ids.last()).y());
                QLineF previousL = QLineF(prevCP1,prevCP2);
                QPointF thisCP = QPointF(controlPoint(original.at(k)).x(),controlPoint(original.at(k)).y());
                QLineF thisL = QLineF(thisCP,new_cpt);
                float angle = std::min(previousL.angleTo(thisL),thisL.angleTo(previousL));
                if(angle>cT) {
                    original.insert(original.begin()+k, addControlPoint(prevCP1));
                    perpendicular_cpts_ids.insert(perpendicular_cpts_ids.begin()+k, addControlPoint(prevCP1,z));
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

                    translated_cpts_ids.push_back(addControlPoint(tmp));
                }
            }

            int cpt_id = addControlPoint(new_cpt);
            translated_cpts_ids.push_back(cpt_id);
        }
    }*/

    QVector<QVector<int> > points;
    /*points.append(original);
    points.append(perpendicular_cpts_ids);
    points.append(translated_cpts_ids);*/
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
