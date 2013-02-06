#include "BSpline.h"
#include "BSplineGroup.h"

BSpline::BSpline():
    idx(-1)
{
}

void BSpline::updatePath()
{
    m_path = QPainterPath();

    int nbPoints = count();
    if(nbPoints>0)
    {
        if(nbPoints==2)
        {
            m_path.moveTo(pointAt(0)) ;
            m_path.lineTo(pointAt(1)) ;
        }
        else
        {
             int i ;
             m_path.moveTo(pointAt(0)) ;
             m_path.lineTo(nextMiddlePoint(0)) ;
             for(i=1; i<(nbPoints-1); i++)
             {
                m_path.quadTo(pointAt(i),nextMiddlePoint(i)) ;
             }
             m_path.lineTo(pointAt(nbPoints-1)) ;
         }
    }
}

QPointF BSpline::pointAt(int index)
{
    int cpt_idx = connected_cpts[index];
    return m_splineGroup->controlPoint(cpt_idx);
}

QPointF BSpline::nextMiddlePoint(int index)
{
    QPointF point1 = pointAt(index) ;
    QPointF point2 = pointAt((index+1)%count()) ;
    return (point1+0.5*(point2-point1)) ;
}
