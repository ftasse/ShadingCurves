#ifndef GLABSTRACT_H
#define GLABSTRACT_H

#include <QGLWidget>
#include <iostream>
#include "3D/point_3d.h"
//#include "../Views/glew/GL/glew.h"

class GLabstract : public QGLWidget
{
    Q_OBJECT

public:
    GLabstract(QWidget *parent = 0, QGLWidget * shareWidget = 0);
    ~GLabstract();

    enum VectorType
    {
        EPS, PDF, PGF
    };

	int 	clr;
//	float 	min, max;

public slots:
	
protected:
    void resizeGL			(int width, int height);

	void genColor(int clr, float x, float minx, float maxx, float col[3]);
	void genColorBW(float x, float minx, float maxx, float col[3]);
	void genColorRG(float x, float minx, float maxx, float col[3]);			
	void genColorHue(float x, float minx, float maxx, float col[3]);
    void genColorHue2(float x, float minx, float maxx, float col[3]);
    void genColorDiscr(float x, float col[3]);

    static const double stepBackAmount;
    static const double frontClipPlane;
    static const double backClipPlane;

	Point_3D screenToWorld (int x, int y);
	Point_3D worldToScreen(Point_3D p);
};

#endif
