#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include <QPoint>
#include <sstream>
#include "BSpline.h"

class Surface
{
public:
    Surface();

    void recompute(cv::Mat dt, int type);
    QVector<QVector<int> > setSurfaceCP(BSpline& bspline, cv::Mat dt, float z, float width, bool inward);
    QPointF traceDT(cv::Mat dt, QPointF point, QPoint current, QLineF normalL, float width);

    // HENRIK: find the closest highest value in neighbourhood
    QPoint localMax(cv::Mat I, cv::Rect N, float *oldD, QLineF normalL, QList<QPoint> visited);

    Point3d& pointAt(int u, int v);
    QVector<QVector<int> > getFaceIndices();

    bool writeOFF(std::ostream &ofs);
    std::string surfaceToOFF()
    {
        std::stringstream ss;
        writeOFF(ss);
        return ss.str();
    }

public:
    BSplineGroup *m_splineGroup;
    int splineRef;
    int type; //Look at type on the attribute class
    int ref;

    QVector<Point3d> vertices;
    QVector<int> sharpCorners;
    QVector< QVector<int> > controlMesh;
};

#endif // SURFACE_H
