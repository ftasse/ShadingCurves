#include <QtGui>
#include <QtOpenGL>
#include "glviewport.h"

const double GLviewport::minScale        = 0.005;
const double GLviewport::scaleDragSpeed  = 1 / 200.0;
const double GLviewport::scaleWheelSpeed = 1 / 500.0;
const double GLviewport::rotateSpeed     = 90.0;

using namespace std;

#define DARK

GLviewport::GLviewport(QWidget *parent, QGLWidget *shareWidget) : GLabstract(parent, shareWidget)
{
    rotIncr       = 90.0 / 12;
	scale         = 1;
    my_scale      = 1;
    my_centre     = Point_3D();
	boundingCube  = 5;
    alpha         = 0.3;

    //    joinTheDarkSide = false;
    joinTheDarkSide = true;

	lightAmbient[0]  = 0.6;
	lightAmbient[1]  = 0.6;
	lightAmbient[2]  = 0.6;
	lightAmbient[3]  = 1;

	lightDiffuse[0]  = 0.4;
	lightDiffuse[1]  = 0.4;
	lightDiffuse[2]  = 0.4;
	lightDiffuse[3]  = 1;

	lightDiffuse1[0]  = 0.8;
	lightDiffuse1[1]  = 0.8;
	lightDiffuse1[2]  = 0.8;
	lightDiffuse1[3]  = 1;

	lightPosition[0] = -1;
	lightPosition[1] = 1;
	lightPosition[2] = 2;
	lightPosition[3] = 0;

	lightPosition1[0] = 1;
	lightPosition1[1] = -1;
	lightPosition1[2] = -2;
	lightPosition1[3] = 0;

    lightPosition2[0] = 0;
    lightPosition2[1] = 0.5;
    lightPosition2[2] = 1;
    lightPosition2[3] = 0;

	lightSpecular[0] = 1;
	lightSpecular[1] = 1;
	lightSpecular[2] = 1;
	lightSpecular[3] = 1;

	col_metal[0] = 0.6;
	col_metal[1] = 0.6;
	col_metal[2] = 0.6;
    col_metal[3] = alpha;

    col_alive[0] = 1;
    col_alive[1] = 1;
    col_alive[2] = 0;
    col_alive[3] = alpha;

    col_dead[0] = 0.2;
    col_dead[1] = 0.2;
    col_dead[2] = 0.4;
    col_dead[3] = alpha;

    col_poi[0] = 1;
    col_poi[1] = 0;
    col_poi[2] = 0;
    col_poi[3] = 0;

    col_feature[0] = 0;
    col_feature[1] = 0;
    col_feature[2] = 1;
    col_feature[3] = 0;

    col_red[0] = 1;
    col_red[1] = 0;
    col_red[2] = 0;
    col_red[3] = 0;

    col_green[0] = 0;
    col_green[1] = 1;
    col_green[2] = 0;
    col_green[3] = 0;

    col_blue[0] = 0;
    col_blue[1] = 0;
    col_blue[2] = 1;
    col_blue[3] = 0;

    resetColours();
}

GLviewport::~GLviewport()
{
}

void GLviewport::resetColours(void)
{
    if (joinTheDarkSide)
    {
        col_edges[0] = 0;
        col_edges[1] = 1;
        col_edges[2] = 0;
        col_edges[3] = 0;

        col_ctrl[0] = 0.8;
        col_ctrl[1] = 0.8;
        col_ctrl[2] = 0;
        col_ctrl[3] = 0;

        col_faces[0] = 0;
        col_faces[1] = 0;
        col_faces[2] = 0;
        col_faces[3] = alpha;

        col_old[0] = 1;
        col_old[1] = 1;
        col_old[2] = 1;
        col_old[3] = 0;

        col_back[0] = 0.5;
        col_back[1] = 0.5;
        col_back[2] = 0.5;
        col_back[3] = 0;
    }
    else
    {
        col_edges[0] = 0;
        col_edges[1] = 0;
        col_edges[2] = 0;
        col_edges[3] = 0;

        col_ctrl[0] = 0;
        col_ctrl[1] = 0;
        col_ctrl[2] = 0;
        col_ctrl[3] = 0;

        col_faces[0] = 1;
        col_faces[1] = 1;
        col_faces[2] = 1;
        col_faces[3] = alpha;

        col_old[0] = 1;
        col_old[1] = 0;
        col_old[2] = 0;
        col_old[3] = 0;

        col_back[0] = 1;
        col_back[1] = 1;
        col_back[2] = 1;
        col_back[3] = 0;
    }

    glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    update();
}

QSize GLviewport::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize GLviewport::sizeHint() const

{
	return QSize(2500,2500);
}

void GLviewport::sphereProject(float x, float y, float (&v)[3])
{
	float r = qMin(width(), height());
	v[0] = (2 * x  - width()) / r;
	v[1] = (height() - 2 * y) / r;
	float d = v[0] * v[0] + v[1] * v[1];
	if (d > 0.5)
	{
		v[2] = 0.5 / sqrt(d);
	}
	else
	{
		v[2] = sqrt(1 - d);
	}
}

void GLviewport::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::RightButton)
	{
		float oldPos[3], newPos[3], axis[3], diff[3];

		sphereProject(lastPos.x(), lastPos.y(), oldPos);
		sphereProject( event->x(),  event->y(), newPos);

		axis[0] = oldPos[1] * newPos[2] - oldPos[2] * newPos[1];
		axis[1] = oldPos[2] * newPos[0] - oldPos[0] * newPos[2];
		axis[2] = oldPos[0] * newPos[1] - oldPos[1] * newPos[0];

		diff[0] = oldPos[0] - newPos[0];
		diff[1] = oldPos[1] - newPos[1];
		diff[2] = oldPos[2] - newPos[2];

		 emit rotChanged(rotateSpeed *
				   sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]),
				   axis[0], axis[1], axis[2]);
	}
	lastPos = event->pos();
}

void GLviewport::wheelEvent(QWheelEvent *event)
{

//    setScale(scale + event->delta() * scaleWheelSpeed / boundingCube);

	emit scaleChanged(scale + event->delta() * scaleWheelSpeed / boundingCube);
}

void GLviewport::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::RightButton)
	{
		emit doubleClicked();
//		cout << "MC" << endl;
	}
}

void GLviewport::setScale(double newScale)
{
//	makeCurrent();

	if (newScale < minScale * (1 / boundingCube))
	{
		newScale = minScale * (1 / boundingCube);
	}

	if (newScale != scale)
	{
		double ratio = newScale / scale;

//        if (ratio > 1.0)
//        {
			ratio *= ratio; // just a fix for closeups
//        }
//        else
//        {
//        	ratio -+ 0.1;
//        }

//cout << ratio << endl;

		// Pre-multiply by scaling matrix
		GLdouble currentMatrix[16] = {0};
		makeCurrent();
		glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
		glLoadIdentity();
        glScaled(ratio, ratio, ratio);
		glMultMatrixd(currentMatrix);

		updateGL();
		scale = newScale;
	}
}

void GLviewport::setRot(double rot, float ax1, float ax2, float ax3)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
	glRotatef(rot, ax1, ax2, ax3);
	glMultMatrixd(currentMatrix);
	updateGL();
}

void GLviewport::setRotXp(void)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
//	glRotatef(rotIncr, 1, 0, 0);
    glTranslatef(10 * my_scale, 0, 0);
	glMultMatrixd(currentMatrix);
	updateGL();
}

void GLviewport::setRotXm(void)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
//	glRotatef(-rotIncr, 1, 0, 0);
    glTranslatef(-10 * my_scale, 0, 0);
	glMultMatrixd(currentMatrix);
	updateGL();
}


void GLviewport::setRotYp(void)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
//	glRotatef(rotIncr, 0, 1, 0);
    glTranslatef(0, 10 * my_scale, 0);
	glMultMatrixd(currentMatrix);
	updateGL();
}

void GLviewport::setRotYm(void)
{
    // Pre-multiply by rotation matrix
    GLdouble currentMatrix[16] = {0};
    makeCurrent();
    glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
    glLoadIdentity();
//	glRotatef(-rotIncr, 0, 1, 0);
    glTranslatef(0, -10 * my_scale, 0);
    glMultMatrixd(currentMatrix);
    updateGL();
}


void GLviewport::setRotZp(void)
{
//	// Pre-multiply by rotation matrix
//	GLdouble currentMatrix[16] = {0};
//	makeCurrent();
//	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
//	glLoadIdentity();
////	glRotatef(rotIncr, 0, 0, 1);
//    glTranslatef(0, 0, 10 * my_scale);
//	glMultMatrixd(currentMatrix);
//	updateGL();
}

void GLviewport::setRotZm(void)
{
//	// Pre-multiply by rotation matrix
//	GLdouble currentMatrix[16] = {0};
//	makeCurrent();
//	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
//	glLoadIdentity();
////	glRotatef(-rotIncr, 0, 0, 1);
//    glTranslatef(0, 0, -10 * my_scale);
//	glMultMatrixd(currentMatrix);
//	updateGL();
}
