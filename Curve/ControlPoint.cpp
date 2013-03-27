#include <stdio.h>
#include <iostream>
#include "ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/BSplineGroup.h"

//These write and read functions must be defined for the serialization in FileStorage to work
static void write(cv::FileStorage& fs, const std::string&, const Attribute& x)
{
    x.write(fs);
}

static void read(const cv::FileNode& node, Attribute& x, const Attribute& default_value = Attribute()){
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

ControlPoint::ControlPoint():
    Point3d(), ref(-1), isSharp(false)
{
    useDefaultAttributes();
}

ControlPoint::ControlPoint(QPointF val):
    Point3d(val.x(), val.y()), ref(-1), isSharp(false)
{
    useDefaultAttributes();
}

ControlPoint::ControlPoint(float x, float y, float z, Attribute _attributes[2]):
    Point3d(x, y, z), ref(-1), isSharp(false)
{
    attributes[0] = _attributes[0];
    attributes[1] = _attributes[1];
}

void ControlPoint::useDefaultAttributes()
{
    attributes[0].direction = INWARD_DIRECTION;
    attributes[0].extent = 20.0;
    attributes[0].height = 30.0;
    attributes[0].shapePointAtr.push_back(QPointF(.4f, .0f) );
    attributes[0].shapePointAtr.push_back(QPointF(.15f, 1.0f) );

    attributes[1].direction = OUTWARD_DIRECTION;
    attributes[1].extent = 20.0;
    attributes[1].height = 30.0;
    attributes[1].shapePointAtr.push_back(QPointF(.4f, .0f));
    attributes[1].shapePointAtr.push_back(QPointF(.15f, 1.0f));

}

void ControlPoint::print()
{
    printf("x:%.2f y:%.2f z:%.2f", x(), y(), z());
    for (int k=0; k<2; ++k)
    {
        Attribute attribute = attributes[k];
        printf("\n\t extent:%.2f height:%.2f \t", attribute.extent, attribute.height);
        for (int l=0; l<attribute.shapePointAtr.size(); ++l)
        {
            printf("\n\t\t lcx:%.2f lcy:%.2f \t", attribute.shapePointAtr[l].x(), attribute.shapePointAtr[l].y());
        }
    }
    printf("\n\n");
    std::cout << std::flush;
}

BSpline& ControlPoint::splineAt(int index)
{
    int spline_ref = splineRefs[index];
    return m_splineGroup->spline(spline_ref);
}

void ControlPoint::write(cv::FileStorage& fs) const                        //Write serialization for this class
{
    fs << "{:" ;
    fs << "x" << x() << "y" << y() << "z" << z() << "ref" << ref  << "isSharp" << (int)isSharp;

    fs << "splineRefs" << "[:";
    for (int i=0; i<num_splines(); ++i)
        fs << splineRefs[i];
    fs << "]";
    fs << "attribute_dir1" << attributes[0];
    fs << "attribute_dir2" << attributes[1];

    fs << "}";
}

void ControlPoint::read(const cv::FileNode& node)                          //Read serialization for this class
{
    setX((float)node["x"]);
    setY((float)node["y"]);
    setZ((float)node["z"]);
    ref = (int)node["ref"];
    isSharp = (int)node["isSharp"];

    cv::FileNode n = node["splineRefs"];                         // Read string sequence - Get node
   {
        cv::FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
        for (; it != it_end; ++it)
            splineRefs.push_back((int)*it);
    }

    attributes[0] = (Attribute)node["attribute_dir1"];
    attributes[1] = (Attribute)node["attribute_dir2"];
}

void Attribute::write(cv::FileStorage& fs) const
{
    fs << "{:";
    fs << "inwardDirection" << (direction==INWARD_DIRECTION) << "extent" << extent << "height" << height;
    fs << "shapePointAtrs" << "[:";
    for (int i=0; i<shapePointAtr.size(); ++i)
    {
        fs << "{:" << "x" << shapePointAtr[i].x() << "y" << shapePointAtr[i].y() << "}";
    }
    fs << "]";
    fs << "}";
}

void Attribute::read(const cv::FileNode& node)
{
    bool inwardDir = (int)node["inwardDirection"];
    if (inwardDir) direction = INWARD_DIRECTION;
    else direction = OUTWARD_DIRECTION;

    extent = (float)node["extent"];
    height = (float)node["height"];

    cv::FileNode n = node["shapePointAtrs"];                         // Read string sequence - Get node
    {
        cv::FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
        for (; it != it_end; ++it)
        {
            cv::FileNode m = *it;
            shapePointAtr.push_back(QPointF((float)m["x"], (float)m["y"]));
        }
    }
}
