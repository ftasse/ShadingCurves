#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include <QPoint>
#include <set>
#include <sstream>
#include "BSpline.h"

class Surface
{
public:
    Surface();

    void recompute(cv::Mat dt, cv::Mat luminance, bool clipHeight);
    QVector<QVector<int> > setSurfaceCP(QVector<ControlPoint> controlPoints,
                                        QVector<QPointF> normals, cv::Mat dt,
                                        bool inward, bool loop,
                                        bool start_has_zero_height=true, bool end_has_zero_height=true);
    QPointF traceDT(cv::Mat dt, QPoint current, float width);

    // HENRIK: find the closest highest value in neighbourhood
    QPoint localMax(cv::Mat I, cv::Rect N, float *oldD);
    float setThresholds(QLineF normal);

    int addVertex(Point3d vertex);
    int addVertex(QPointF point, float z = 0.0f);
    Point3d& pointAt(int u, int v);
    void computeFaceIndices();

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
    std::set<int> sharpCorners;
    QVector< QVector<int> > controlMesh;
    QVector<QVector<int> > faceIndices;
};

#endif // SURFACE_H
