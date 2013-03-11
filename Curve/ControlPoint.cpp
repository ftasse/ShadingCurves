#include <stdio.h>
#include <iostream>
#include "ControlPoint.h"
#include "../Curve/BSpline.h"
#include "../Curve/BSplineGroup.h"

ControlPoint::ControlPoint():
    Point3d(), ref(-1)
{
    useDefaultAttributes();
}

ControlPoint::ControlPoint(QPointF val):
    Point3d(val.x(), val.y()), ref(-1)
{
    useDefaultAttributes();
}

ControlPoint::ControlPoint(float x, float y, float z, Attribute _attributes[2]):
    Point3d(x, y, z), ref(-1)
{
    attributes[0] = _attributes[0];
    attributes[1] = _attributes[1];
}

void ControlPoint::useDefaultAttributes()
{
    attributes[0].direction = INWARD_DIRECTION;
    attributes[0].extent = 50.0;
    attributes[0].height = 50.0;
    attributes[0].shapePointAtr.push_back(QPointF(0.6f, 0.0f) );
    attributes[0].shapePointAtr.push_back(QPointF(.5f, .9f) );

    attributes[1].direction = OUTWARD_DIRECTION;
    attributes[1].extent = 50.0;
    attributes[1].height = -50.0;
    attributes[1].shapePointAtr.push_back(QPointF(0.6f, 0.0f));
    attributes[1].shapePointAtr.push_back(QPointF(.5f, .9f));

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
