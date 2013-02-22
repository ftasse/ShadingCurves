#include <QtGui>
#include <QtOpenGL>
#include "GL/glu.h"
#include <cmath>
#include <math.h>
#include "glabstract.h"

const double GLabstract::stepBackAmount  = 10.0;
const double GLabstract::frontClipPlane  = -1000;
const double GLabstract::backClipPlane   = 1000;

using namespace std;

GLabstract::GLabstract(QWidget *parent, QGLWidget *shareWidget) : QGLWidget(parent, shareWidget)
{
    clr = 0;
//	min = 0;
//	max = 0;	
}

GLabstract::~GLabstract()
{
}

void GLabstract::resizeGL(int width, int height)
{
    makeCurrent();
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double dbMin = 2 * qMin(width, height);
    glOrtho(-width  / dbMin, width  / dbMin,
            -height / dbMin, height / dbMin, frontClipPlane, backClipPlane);
    glMatrixMode(GL_MODELVIEW);
}

void GLabstract::genColor(int clr, float x, float minx, float maxx, float col[3])
{

//	if (x > 0) x = floor(x);
//	else x = ceil(x);

	if (x > maxx) x = maxx;
	if (x < minx) x = minx;

	if (clr == 0)
	{
		genColorHue(x, minx, maxx, col);
	}
	else if (clr == 1)
	{
		genColorRG(x, minx, maxx, col);
	}
	else if (clr == 2)
	{
        genColorDiscr(x, col);
	}
    else if (clr == 3)
	{
		genColorBW(x, minx, maxx, col);
	}
    else
    {
        genColorHue2(x, minx, maxx, col);
    }
}

void GLabstract::genColorDiscr(float x, float col[3])
{
//	float value;
////	value = (x - minx) / (maxx - minx) - 0.5;
//    value = x;
	
////	float eps = 0.01;    //- for spline based curvG
////    float eps = 0.0;     //- for spline based curvG
//    float eps = 0.00008; //- for angle based curvG
////    float eps = (maxx - minx) / 500.0;

//    if (value > eps)
//	{
//		col[0] = 1;
//		col[1] = 0;
//		col[2] = 0;
//	}
//    else if (value < -eps)
//	{
//		col[0] = 0;
//		col[1] = 0;
//		col[2] = 1;
//	}
//	else
//	{
//		col[0] = 0;
//		col[1] = 1;
//		col[2] = 0;
//	}

    if (x >= 0.1)
    {
        col[0] = 1;
        col[1] = 0;
        col[2] = 0;
    }
    else if (x <= -0.1)
    {
        col[0] = 0;
        col[1] = 0;
        col[2] = 1;
    }
    else
    {
        col[0] = 0;
        col[1] = 1;
        col[2] = 0;
    }
}

void GLabstract::genColorBW(float x, float minx, float maxx, float col[3])
{
	float value;

    value = (x - minx) / (maxx - minx);
	col[0] = value;
	col[1] = value;
	col[2] = value;
}

void GLabstract::genColorRG(float x, float minx, float maxx, float col[3])
{
	float value;

	value = (x - minx) / (maxx - minx);
	col[0] = value;
	col[1] = 1.0 - value;
	col[2] = 0;
}

void GLabstract::genColorHue(float x, float minx, float maxx,  float col[3])
{
    float H, var_h, var_i, var_r, var_g, var_b, up, up0, up1, dw, dw0, dw1, lin, lin2, fac;

	if (maxx == minx)
	{
		H = 0.5;
	}
	else
	{
		H = (x - minx) / (maxx - minx);
	}

    var_h = 5 - H * 5;
	var_i = floor( var_h );
    lin = var_h - var_i;
//    up = lin; // linear
    up = -2.0 * lin * lin * lin + 3.0 * lin * lin; // cubic
    dw = 1 - up;
   	
    up0 = -0.25 * lin * lin * lin + 0.75 * lin * lin;
    lin2 = (lin + 1);
    up1 = -0.25 * lin2 * lin2 * lin2 + 0.75 * lin2 * lin2;
    dw0 = 1 - up1;
    dw1 = 1 - up0;
   	
    if ( var_i == 0 )
    {
    	var_r = 1;
    	var_g = up;
    	var_b = 0;
    }
    else if ( var_i == 1 )
    {
    	var_r = dw;
    	var_g = 1;
    	var_b = 0;
    }
    else if ( var_i == 2 )
    {
    	var_r = 0;
    	var_g = 1;
    	var_b = up;
    }
    else if ( var_i == 3 )
    {
    	var_r = up0;
    	var_g = dw1;
    	var_b = 1;
    }
   	else
   	{
   		var_r = up1;
   		var_g = dw0;
   		var_b = 1;
   	}

    if ( H == 0 )
	{
   		col[0] = 1;
        col[1] = 0;
   		col[2] = 1;
	}
    else if ( H == 1)
    {
        col[0] = 1;
        col[1] = 0;
        col[2] = 0;
    }
	else
	{
   		col[0] = var_r;
   		col[1] = var_g;
   		col[2] = var_b;
   	}
}

void GLabstract::genColorHue2(float x, float minx, float maxx,  float col[3])
{
    float H, var_h, var_i, var_r, var_g, var_b, up, up0, up1, dw, dw0, dw1, lin;

    if (maxx == minx)
    {
        H = 0.5;
    }
    else
    {
        H = (x - minx) / (maxx - minx);
    }

    var_h = 4 - H * 4;
    var_i = floor( var_h );
    lin = var_h - var_i;
//    up = lin; // linear
    up = -2.0 * lin * lin * lin + 3.0 * lin * lin; // cubic
    dw = 1 - up;

//    up0 = up / 2;
//    up1 = 0.5 + up0;
//    dw0 = dw / 2;
//    dw1 = 0.5 + dw0;

   if ( var_i == 0 )
    {
        var_r = 1;
        var_g = up;
        var_b = 0;
    }
    else if ( var_i == 1 )
    {
        var_r = dw;
        var_g = 1;
        var_b = 0;
    }
    else if ( var_i == 2 )
    {
        var_r = 0;
        var_g = 1;
        var_b = up;
    }
    else if ( var_i == 3 )
    {
        var_r = 0;
        var_g = dw;
        var_b = 1;
    }
    else
    {
        var_r = up;
        var_g = 0;
        var_b = 1;
    }

    if (var_h == 4)
    {
        col[0] = 0;
        col[1] = 0;
        col[2] = 1;
    }
    else
    {
        col[0] = var_r;
        col[1] = var_g;
        col[2] = var_b;
    }
}

/// Returns the world coordinates of a point in screen space.
/// @param p input point in world space
/// @return point in screen space
Point_3D GLabstract::screenToWorld (int x, int y) 
{
    makeCurrent();
    int viewport[4];
    GLdouble mvmatrix[16], projmatrix[16], wldcoord[3];
    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

    gluUnProject (x, y, 0, mvmatrix, projmatrix, viewport, &wldcoord[0], &wldcoord[1], &wldcoord[2]);

    return Point_3D(wldcoord[0], wldcoord[1], wldcoord[2]);
}

/// Returns the screen coordinates of a point in world space.
/// @param p input point in screen space
/// @return point in world space
Point_3D GLabstract::worldToScreen(Point_3D p)
{
    makeCurrent();
    int viewport[4];
    double mvmatrix[16], projmatrix[16], wincoord[3];
    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
    gluProject (p.getX(), p.getY(), p.getZ(),
               mvmatrix, projmatrix, viewport,
               &wincoord[0], &wincoord[1], &wincoord[2]);
    return Point_3D(wincoord[0], wincoord[1], wincoord[2]);
}
