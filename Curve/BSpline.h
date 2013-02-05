#ifndef BSPLINE_H
#define BSPLINE_H

#include <vector>
#include "Curve/ControlPoint.h"

class BSpline
{
public:
    BSpline();

    std::vector<int>& controlPointsRefs()
    {
        return m_cptsRefs;;
    }

    ControlPoint pointAt(int i)
    {
        return (*cptsPtr)[ m_cptsRefs[i] ];
    }

public:
    std::vector<ControlPoint> *cptsPtr;

private:
    std::vector<int> m_cptsRefs;
};

#endif // BSPLINE_H
