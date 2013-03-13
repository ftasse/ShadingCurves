#ifndef SHADINGPROFILEITEM_H
#define SHADINGPROFILEITEM_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QSlider>
#include "Curve/ControlPoint.h"

class ShadingProfileView : public QWidget
{
    Q_OBJECT

public:
    ShadingProfileView();

    void setControlPoint(ControlPoint &cpt)
    {
        m_controlPoint = &cpt;
        updatePath();
    }

    ControlPoint getControlPoint()
    {
        return *m_controlPoint;
    }


private:
    ControlPoint* m_controlPoint;

    QGraphicsView* graphicsView;
    QSlider *inwardExtentWidget;
    QSlider *outwardExtentWidget;

    QSlider *inwardHeightWidget;
    QSlider *outwardHeightWidget;

signals:
    void controlPointAttributesChanged(int cptRef);

public slots:

    void updatePath();
    void updateControlPointParameters();
};

#endif // SHADINGPROFILEITEM_H
