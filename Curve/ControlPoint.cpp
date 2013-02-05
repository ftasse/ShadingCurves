#include "ControlPoint.h"

ControlPoint::ControlPoint(float x, float y) :
    std::vector<float>(2, 0.0)
{
    this->data()[0] = x;
    this->data()[1] = y;
}

float ControlPoint::x()
{
    return this->data()[0];
}

float ControlPoint::y()
{
    return this->data()[1];
}
