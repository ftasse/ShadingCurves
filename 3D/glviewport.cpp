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

        col_ctrl[0] = 1;
        col_ctrl[1] = 1;
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

        col_back[0] = 0;
        col_back[1] = 0;
        col_back[2] = 0;
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

void GLviewport::initializeGL(void)
{
	static const int res = 1024;
	PointPrec		col[3];
    GLfloat			texture[5][res][3];

//    if (joinTheDarkSide)
//    {
//        glClearColor(0, 0, 0, 0);
//    }
//    else
//    {
//        glClearColor(1, 1, 1, 0);
//    }

    glClearColor(col_back[0], col_back[1], col_back[2], col_back[3]);

//    // FOG
//    fogColor[0] = 0.5;
//    fogColor[1] = 0.5;
//    fogColor[2] = 0.5;
//    fogColor[3] = 1.0;
//    glClearColor(0.f,0.f,0.f,1.0f);  // Clear To The Color Of The Fog
//    glFogi(GL_FOG_MODE, GL_EXP);        // Fog Mode: GL_EXP, GL_EXP2, GL_LINEAR
//    glFogfv(GL_FOG_COLOR, fogColor);    // Set Fog Color
//    glFogf(GL_FOG_DENSITY, 0.3f);      // How Dense Will The Fog Be
//    glHint(GL_FOG_HINT, GL_NICEST);  // Fog Hint Value
////    glFogf(GL_FOG_START, 0.0f);         // Fog Start Depth
////    glFogf(GL_FOG_END, 1.0f);           // Fog End Depth
//    glEnable(GL_FOG);                   // Enables GL_FOG
//    // END FOG

	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
//    glDepthMask(GL_FALSE);
//    glEnable(GL_CULL_FACE);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  lightDiffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glEnable (GL_LIGHT1);
    glLightfv(GL_LIGHT2, GL_POSITION, lightPosition2);
    glLightfv(GL_LIGHT2, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  lightDiffuse1);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular);
//    glEnable(GL_LIGHT2);

	glLoadIdentity();
	swapBuffers();

    glGenTextures(6, textID);

    for (int j = 0; j < 5; ++j) // number of colour systems
	{
		for (int i = 0; i < res; ++i)
		{
            genColor(j, (float)i / res - 0.5, -0.5, 0.5, col);

			texture[j][i][0] = col[0];
			texture[j][i][1] = col[1];
			texture[j][i][2] = col[2];
		}

		glBindTexture(GL_TEXTURE_1D, textID[j]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glDisable(GL_TEXTURE_GEN_S);

		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, res,
					 0, GL_RGB, GL_FLOAT, texture[j]);
	}
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
	glRotatef(rotIncr, 1, 0, 0);
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
	glRotatef(-rotIncr, 1, 0, 0);
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
	glRotatef(rotIncr, 0, 1, 0);
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
	glRotatef(-rotIncr, 0, 1, 0);
	glMultMatrixd(currentMatrix);
	updateGL();
}


void GLviewport::setRotZp(void)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
	glRotatef(rotIncr, 0, 0, 1);
	glMultMatrixd(currentMatrix);
	updateGL();
}

void GLviewport::setRotZm(void)
{
	// Pre-multiply by rotation matrix
	GLdouble currentMatrix[16] = {0};
	makeCurrent();
	glGetDoublev(GL_MODELVIEW_MATRIX, currentMatrix);
	glLoadIdentity();
	glRotatef(-rotIncr, 0, 0, 1);
	glMultMatrixd(currentMatrix);
	updateGL();
}

void GLviewport::setRotZero()
{
	makeCurrent();
//	glLoadIdentity();
//	glScalef(0.7,0.7,0.7);
//	scale = 1;
    glLoadIdentity();
    glScalef(0.7 * my_scale, 0.7 * my_scale, 0.7 * my_scale);
    glTranslatef(-my_centre.getX(), -my_centre.getY(), -my_centre.getZ());
    scale = 1;

    updateGL();
}
