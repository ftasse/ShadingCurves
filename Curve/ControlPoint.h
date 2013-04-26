#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <assert.h>
#include <QList>
#include <QVector>
#include <opencv2/core/core.hpp>

#include "Point3d.h"

class BSplineGroup;
class BSpline;

typedef enum NormalDirection
{
  INWARD_DIRECTION = 0,
  OUTWARD_DIRECTION
} NormalDirection;

class Attribute
{
public:
    NormalDirection direction; //0 for inward point along the normal,    1 for outward point along the normal
    float extent;
    float height;
    QVector<QPointF> shapePointAtr;

    Attribute()
    {
        direction = INWARD_DIRECTION;
        extent = height = 0.0f;
    }

    Attribute(cv::FileNode node)//:Attribute()
    {
        direction = INWARD_DIRECTION;
        extent = height = 0.0f;
        this->read(node);
    }

    void write(cv::FileStorage& fs) const ;
    void read(const cv::FileNode& node);
};

class ControlPoint : public Point3d
{
public:
    ControlPoint();
    ControlPoint(QPointF val);
    ControlPoint(float x, float y, float z, Attribute attributes[2]);

    ControlPoint(cv::FileNode node):Point3d(), ref(-1), isSharp(false)//:ControlPoint()
    {
        useDefaultAttributes();
        read(node);
    }

    BSpline& splineAt(int index);

    int num_splines()   const
    {
        return splineRefs.size();
    }

    Attribute& attribute(NormalDirection dir)
    {
        if (dir == INWARD_DIRECTION)
            return attributes[0];
        else
            return attributes[1];
    }

    void useDefaultAttributes();
    void print();

    void write(cv::FileStorage& fs) const ;
    void read(const cv::FileNode& node);

public:
    BSplineGroup *m_splineGroup;
    QVector<int> splineRefs;

    int ref;
    bool isSharp;
    Attribute attributes[2];
};


inline ControlPoint operator+(ControlPoint p1, ControlPoint p2)
{
    Attribute new_attributes[2];
    for (int k=0; k<2; ++k)
    {
        new_attributes[k].direction = p1.attributes[k].direction;
        new_attributes[k].height = p1.attributes[k].height + p2.attributes[k].height;
        new_attributes[k].extent = p1.attributes[k].extent + p2.attributes[k].extent;

        assert(p1.attributes[k].shapePointAtr.size() == p2.attributes[k].shapePointAtr.size());
        for (int l=0; l<p1.attributes[k].shapePointAtr.size(); ++l)
        {
            new_attributes[k].shapePointAtr.push_back(p1.attributes[k].shapePointAtr[l] + p2.attributes[k].shapePointAtr[l]);
        }
    }
    ControlPoint cpt(p1.x()+p2.x(), p1.y()+p2.y(), p1.z()+p2.z(), new_attributes);
    return cpt;
}

inline ControlPoint operator*(qreal a, ControlPoint p2)
{
    Attribute new_attributes[2];
    for (int k=0; k<2; ++k)
    {
        new_attributes[k].direction = p2.attributes[k].direction;
        new_attributes[k].height = a*p2.attributes[k].height;
        new_attributes[k].extent = a*p2.attributes[k].extent;

        for (int l=0; l<p2.attributes[k].shapePointAtr.size(); ++l)
        {
            new_attributes[k].shapePointAtr.push_back(a*p2.attributes[k].shapePointAtr[l]);
        }
    }
    ControlPoint cpt(a*p2.x(), a*p2.y(), a*p2.z(), new_attributes);
    return cpt;
}

inline Attribute operator+(Attribute a1, Attribute a2)
{
    Attribute new_attribute;
    new_attribute.direction = a1.direction;
    new_attribute.height = a1.height + a2.height;
    new_attribute.extent = a1.extent + a2.extent;

    assert(a1.shapePointAtr.size() == a2.shapePointAtr.size());
    for (int l=0; l<a1.shapePointAtr.size(); ++l)
    {
        new_attribute.shapePointAtr.push_back(a1.shapePointAtr[l] + a2.shapePointAtr[l]);
    }
    return new_attribute;
}

inline Attribute operator*(qreal s, Attribute a2)
{
    Attribute new_attribute;
    new_attribute.direction = a2.direction;
    new_attribute.height = s*a2.height;
    new_attribute.extent = s*a2.extent;

    for (int l=0; l<a2.shapePointAtr.size(); ++l)
    {
        new_attribute.shapePointAtr.push_back(s*a2.shapePointAtr[l]);
    }
    return new_attribute;
}

#endif // CONTROLPOINT_H
