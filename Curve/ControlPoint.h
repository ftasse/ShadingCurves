#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <vector>

class ControlPoint : public std::vector<float>
{
public:
    ControlPoint(float x = 0.0f, float y = 0.0f);

    float x();
    float y();
};

#endif // CONTROLPOINT_H
