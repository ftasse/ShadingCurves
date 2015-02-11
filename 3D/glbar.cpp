#include <QtGui>
#include <QtOpenGL>
#include "glbar.h"

using namespace std;

GLbar::GLbar(QWidget *parent) : GLabstract(parent)
{
    bar_enabled = true;
    clr = 3;
}

GLbar::~GLbar()
{
}

QSize GLbar::minimumSizeHint() const
{
    return QSize(120, 30);
}

QSize GLbar::sizeHint() const

{
    return QSize(120, 30);
}

void GLbar::initializeGL(void)
{
    glClearColor(1, 1, 1, 0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glLoadIdentity();
    buildBar();
}

void GLbar::paintGL(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (bar_enabled)
	{
		glCallList(bar_list);
	}
}

void GLbar::drawBar()
{
	float 		col[3];
	int 		i, maxi;
	float 		step = 0.05;
	Point_3D 	poi;

    float 	minx = -2;
    float 	maxx = 2;
    float 	miny = -1;
    float 	maxy = 1;

    maxi = (int) (maxx - minx) / step;

    for (i = 0 ; i < maxi ; i++)
    {
        glBegin(GL_POLYGON);
        genColor(clr, i - maxi * 0.5, - maxi * 0.5, maxi * 0.5 - 1, col);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);

        glVertex3f(minx + i * step, miny, 0.0);
        glVertex3f(minx + (i + 1) * step, miny, 0.0);
        glVertex3f(minx + (i + 1) * step, maxy, 0.0);
        glVertex3f(minx + i * step, maxy, 0.0);

        glEnd();
    }
}

void GLbar::buildBar()
{
	makeCurrent();
	if(bar_list) glDeleteLists(bar_list, 1);
    bar_list = glGenLists (1);
    glNewList (bar_list, GL_COMPILE);
		glPushAttrib(GL_CURRENT_BIT);
//		glDepthRange (0.1, 1.0);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
//		glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawBar();

		glPopAttrib();
	glEndList();
}

void GLbar::setBar(int color)
{
	clr = color;
	buildBar();
	updateGL();
}
