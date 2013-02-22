#ifndef GLVIEWPORT_H
#define GLVIEWPORT_H

#include <QGLWidget>
#include <QSize>
#include <iostream>
#include "3D/glabstract.h"
#include "3D/point_3d.h"

class GLviewport : public GLabstract
{
	Q_OBJECT

public:
	GLviewport(QWidget *parent = 0, QGLWidget * shareWidget = 0);
	~GLviewport();

	enum DrawMeshType
	{
        FLAT, SMOOTH, WIREFRAME, SOLIDFRAME, MEAN, GAUSSIAN, TGAUSSIAN, HEIGHT, ISOPHOTES
	};

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
    GLuint			textID[6];


    bool        joinTheDarkSide;

public slots:
	void mouseMoveEvent		(QMouseEvent *event);
	void wheelEvent			(QWheelEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void setScale           (double newScale);
	void setRot             (double rot, float ax1, float ax2, float ax3);
	void setRotXp			(void);
	void setRotYp			(void);
	void setRotZp			(void);
	void setRotXm			(void);
	void setRotYm			(void);
	void setRotZm			(void);

	virtual void buildAll			(void) = 0;

    void resetColours(void);

signals:
	void scaleChanged(double newScale);
	void rotChanged(double rot, float ax1, float ax2, float ax3);
	void doubleClicked();

protected:
	void sphereProject      (float x, float y, float (&v)[3]);
//	void calculateNormal(GLfloat *norm, GLfloat p0[3],
//	                     GLfloat p1[3], GLfloat p2[3]);

	float         boundingCube;
    float         scale, my_scale;
	QPoint        lastPos;
	float		  rotIncr;
    Point_3D       my_centre;

	static const double minScale;
	static const double scaleDragSpeed;
	static const double scaleWheelSpeed;
	static const double rotateSpeed;

	float      	lightAmbient[4];
	float       lightDiffuse[4];
	float       lightDiffuse1[4];
	float       lightSpecular[4];
	float       lightPosition[4];
	float       lightPosition1[4];
    float       lightPosition2[4];

	float 		alpha;

    GLfloat 	col_metal[4];
    GLfloat 	col_alive[4];
    GLfloat 	col_dead[4];
    GLfloat		col_edges[4];
    GLfloat		col_ctrl[4];
    GLfloat		col_old[4];
    GLfloat		col_faces[4];
    GLfloat		col_poi[4];
    GLfloat		col_feature[4];
    GLfloat		col_back[4];


//    GLfloat     fogColor[4];

};

#endif
