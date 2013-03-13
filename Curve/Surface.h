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

    void recompute(cv::Mat dt);
    QVector<QVector<int> > setSurfaceCP(QVector<ControlPoint> controlPoints, cv::Mat dt, bool inward);
    QPointF traceDT(cv::Mat dt, QPointF point, QPoint current, QLineF normalL, float width);

    // HENRIK: find the closest highest value in neighbourhood
    QPoint localMax(cv::Mat I, cv::Rect N, float *oldD, QLineF normalL, QList<QPoint> visited, float Td, float Ta);
    float setThresholds(QLineF normal);

    int addVertex(Point3d vertex);
    int addVertex(QPointF point, float z = 0.0f);
    Point3d& pointAt(int u, int v);
    QVector<QVector<int> > getFaceIndices();

    bool writeOFF(std::ostream &ofs);
    std::string surfaceToOFF( cv::Vec3b color)
    {
        std::stringstream ss;
        writeOFF(ss);
        ss << (int)color(0) << " " << (int)color(1) << " " << (int)color(2) << "\n";
        return ss.str();
    }

public:
    BSplineGroup *m_splineGroup;
    int splineRef;
    NormalDirection direction; //Look at type on the attribute class
    int ref;

    QVector<Point3d> vertices;
    QVector<int> sharpCorners;
    QVector< QVector<int> > controlMesh;
};

#endif // SURFACE_H
